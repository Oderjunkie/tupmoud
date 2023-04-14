#include <stdio.h>
#include <curl/curl.h>
#include <git2.h>
#include <string.h>
#include <stdlib.h>
#include "autobuild.h"
#include "install.h"
#include "jsmn.h"

int jsoneq(const char *json, jsmntok_t tok, const char *s) {
  if (tok.type == JSMN_STRING && (int) strlen(s) == tok.end - tok.start &&
      strncmp(json + tok.start, s, tok.end - tok.start) == 0) {
    return 0;
  }
  return -1;
}

void skip(jsmntok_t *tokv, int tokc, int *i) {
  int after;
  (*i)++;
  after = tokv[*i].end;
  while (tokv[*i].start <= after && *i < tokc)
    (*i)++;
}

void empstr(char **str) {
  *str = malloc(1);
  *str[0] = '\0';
}

int insocb(char *data, size_t size, size_t nmemb) {
  jsmn_parser p;
  jsmntok_t *tokv;
  int ktokn, keyn, keyc, tokc;
  static char *buf = NULL;
  static size_t buflen = 0;
  tokv = malloc(128 * sizeof(*tokv));
  tokc = 128;
  {
    char *newbuf;
    newbuf = malloc(buflen + size * nmemb);
    if (newbuf == NULL) {
      printf("OOM!\n");
      free(buf);
      exit(EXIT_FAILURE);
    }
    if (buf != NULL)
      memcpy(newbuf, buf, buflen);
    memcpy(newbuf + buflen, data, size * nmemb);
    free(buf);
    buflen += size * nmemb;
    buf = newbuf;
  }
  while ((jsmn_init(&p), keyc = jsmn_parse(&p, buf, buflen, tokv, tokc)) == JSMN_ERROR_NOMEM) {
    tokc += 128;
    if (tokv != NULL)
      free(tokv);
    tokv = malloc((tokc + 1) * sizeof(*tokv));
    if (tokv == NULL) {
      printf("OOM!\n");
      exit(EXIT_FAILURE);
    }
  }
  if (keyc < 0) {
    free(tokv);
    return(size * nmemb);
  }
  ktokn = 1;
  for (keyn = 0; keyn < keyc && keyn < tokc; keyn++) {
    if (!jsoneq(buf, tokv[ktokn], "items")) {
      int rtokn, repon;
      ktokn++;
      rtokn = 1;
      for (repon = 0; repon < tokv[ktokn].size && ktokn+rtokn < tokc; repon++) {
        char *name, *url, *lic;
        int k, k1;
        empstr(&name);
        empstr(&url);
        empstr(&lic);
        k = 1;
        for (k1 = 0; k1 < tokv[ktokn+rtokn].size && ktokn+rtokn+k < tokc; k1++) {
          if (!jsoneq(buf, tokv[ktokn+rtokn+k], "full_name")) {
            int len;
            k++;
            len = tokv[ktokn+rtokn+k].end - tokv[ktokn+rtokn+k].start;
            free(name);
            name = malloc(len + 1);
            memcpy(name, buf + tokv[ktokn+rtokn+k].start, len);
            name[len] = '\0';
            k++;
          } else if (!jsoneq(buf, tokv[ktokn+rtokn+k], "clone_url")) {
            int len;
            k++;
            len = tokv[ktokn+rtokn+k].end - tokv[ktokn+rtokn+k].start;
            free(url);
            url = malloc(len + 1);
            memcpy(url, buf + tokv[ktokn+rtokn+k].start, len);
            url[len] = '\0';
            k++;
          } else if (!jsoneq(buf, tokv[ktokn+rtokn+k], "license")) {
            int l, l1;
            k++;
            l = 1;
            for (l1 = 0; l1 < tokv[ktokn+rtokn+k].size; l1++) {
              if (!jsoneq(buf, tokv[ktokn+rtokn+k+l], "key")) {
                int len;
                l++;
                len = tokv[ktokn+rtokn+k+l].end - tokv[ktokn+rtokn+k+l].start;
                free(lic);
                lic = malloc(len + 1);
                memcpy(lic, buf + tokv[ktokn+rtokn+k+l].start, len);
                lic[len] = '\0';
                l++;
              } else {
                skip(tokv+ktokn+rtokn+k, tokc, &l);
              }
            }
            skip(tokv+ktokn+rtokn, tokc, &k);
          } else {
            skip(tokv+ktokn+rtokn, tokc, &k);
          }
        }
        printf("cloning \x1b[31m%s\x1b[0m... (at %s)\n", name, url);
        free(lic);
        {
          git_repository *repo = NULL;
          char *tmpdir;
          tmpdir = malloc(sizeof("/tmp/") + strlen(name) + 1);
          sprintf(tmpdir, "/tmp/%s", name);
          {
            char *tmpcmd;
            tmpcmd = malloc(sizeof("rm -rf /tmp/") + strlen(name) + 1);
            sprintf(tmpcmd, "rm -rf /tmp/%s", name);
            if (!system(tmpcmd)) {} else {};
            free(tmpcmd);
          }
          git_clone(&repo, url, tmpdir, NULL);
          git_repository_free(repo);
          printf("building...\n");
          autbld(tmpdir);
          free(tmpdir);
        }
        free(name);
        free(url);
        free(tokv);
        free(buf);
        buf = NULL;
        return(size * nmemb);
      }
    } else {
      skip(tokv, tokc, &ktokn);
    }
  }
  free(tokv);
  free(buf);
  buf = NULL;
  return(size * nmemb);
}

int insone(char *pkg) {
  CURL *session;
  printf("searching for %s...\n", pkg);
  if((session = curl_easy_init())) {
    struct curl_slist *list = NULL;
    CURLcode res;
    char *buf;
    buf = malloc(sizeof("https://api.github.com/search/repositories?q=") + strlen(pkg) + 1);
    sprintf(buf, "https://api.github.com/search/repositories?q=%s", pkg);
    
    curl_easy_setopt(session, CURLOPT_URL, buf);
    curl_easy_setopt(session, CURLOPT_WRITEFUNCTION, insocb);
    
    // yes this is a github api key
    // no it is not a security vuln.
    list = curl_slist_append(list, "Authorization: Bearer github_pat_11AOBHFJI"
    "0lBtSj4OgzcPB_7d5ZFCmtjbZvvuE2rnzPWVQPnjKfqGMQOBqgsXTLpQnDHTTNKUU3FVGsRJk"
    );
    list = curl_slist_append(list, "Accept: application/vnd.github+json");
    list = curl_slist_append(list, "User-Agent: curl");
    curl_easy_setopt(session, CURLOPT_HTTPHEADER, list);
    
    res = curl_easy_perform(session);
    curl_easy_cleanup(session);
    curl_slist_free_all(list);
    free(buf);
    if (res != CURLE_OK) {
      printf("%s\n", curl_easy_strerror(res));
      exit(EXIT_FAILURE);
    }
  }
  return(0);
}

int insall(int pkgc, char *pkgv[]) {
  int i;
  git_libgit2_init();
  for (i = 0; i < pkgc; i++)
    insone(pkgv[i]);
  printf("done!\n");
  git_libgit2_shutdown();
  return(0);
}
