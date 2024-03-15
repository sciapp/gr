#define _POSIX_C_SOURCE 200809L
#define _XOPEN_SOURCE 500

#include <assert.h>
#include <ftw.h>
#include <signal.h>
#include <string.h>
#include <grm.h>
#include "grm/memwriter_int.h"


#if (defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 5))) || defined(__clang__)
#define UNUSED \
  __attribute__((unused, deprecated("Marked as \"UNUSED\" but used. Please remove the \"UNUSED\" marker.")))
#else
#define UNUSED
#endif

static const char *tmp_dir = NULL;

static char *create_tmp_dir(void)
{
  char *tmp_dir_;
  const char *dirname_template, *system_tmp_dir;

  dirname_template = "grm.XXXXXX";
  system_tmp_dir = getenv("TMPDIR");
  if (system_tmp_dir == NULL)
    {
      system_tmp_dir = "/tmp";
    }
  tmp_dir_ = malloc(strlen(system_tmp_dir) + strlen(dirname_template) + 2);
  if (tmp_dir_ == NULL)
    {
      return NULL;
    }
  sprintf(tmp_dir_, "%s/%s", system_tmp_dir, dirname_template);
  if (mkdtemp(tmp_dir_) == NULL)
    {
      free(tmp_dir_);
      return NULL;
    }

  tmp_dir = tmp_dir_;
  return tmp_dir_;
}

static int remove_callback(const char *fpath, const struct stat *sb UNUSED, int typeflag UNUSED,
                           struct FTW *ftwbuf UNUSED)
{
  int rv = remove(fpath);
  fprintf(stderr, "Removed \"%s\"\n", fpath);

  if (rv) perror(fpath);

  return rv;
}

static void cleanup(void)
{
  if (tmp_dir == NULL) return;
  nftw(tmp_dir, remove_callback, 64, FTW_DEPTH | FTW_PHYS);
  free((void *)tmp_dir);
  tmp_dir = NULL;
}

static void signal_handler(int sig UNUSED)
{
  cleanup();
}

static void test_bson()
{
  int i = 1986;
  double d = 5.05;
  char c = 'y';
  int y_i[] = {1986, 1986, 1986};
  double y_d[] = {5.05, 5.05, 5.05};
  char *y_s[] = {"eins", "zwei", "drei"};
  char y_c[] = {'a', 'b', 'c', '\0'};
  grm_args_t *args, *subarg, *subargs[2], *read_args;
  char *filepath, *bson_buffer;
  memwriter_t *memwriter;
  FILE *bson_file;
  int bson_buffer_size;
  size_t bson_file_size, bytes_read;
  err_t error;

  args = grm_args_new();
  subarg = grm_args_new();
  subargs[0] = grm_args_new();
  subargs[1] = grm_args_new();

  grm_args_push(args, "y_i", "i", i);
  grm_args_push(args, "y_d", "d", d);
  grm_args_push(args, "y_s", "s", y_s[0]);
  grm_args_push(args, "y_c", "c", c);

  grm_args_push(args, "y_I", "nI", sizeof(y_i) / sizeof(y_i[0]), y_i);
  grm_args_push(args, "y_D", "nD", sizeof(y_d) / sizeof(y_d[0]), y_d);
  grm_args_push(args, "y_S", "nS", sizeof(y_s) / sizeof(y_s[0]), y_s);
  grm_args_push(args, "y_C", "nC", 4, y_c);

  grm_args_push(subarg, "hello", "s", "world");
  grm_args_push(args, "sub", "a", subarg);

  grm_args_push(subargs[0], "hello", "s", "world");
  grm_args_push(subargs[1], "y", "d", d);
  grm_args_push(args, "subs", "nA", 2, subargs);

  printf("Argument container to be serialized as BSON:\n");
  grm_dump(args, stdout);

  printf("\nArgument container as BSON:\n");
  grm_dump_bson(args, stdout);

  filepath = malloc(strlen(tmp_dir) + strlen("bson.bytes") + 2);
  assert(filepath != NULL);

  sprintf(filepath, "%s/%s", tmp_dir, "bson.bytes");
  bson_file = fopen(filepath, "wb");
  assert(bson_file != NULL);

  memwriter = memwriter_new();
  assert(memwriter != NULL);
  tobson_write_args(memwriter, args);
  assert(tobson_is_complete());
  bson_buffer = memwriter_buf(memwriter);
  bytes_to_int(&bson_buffer_size, bson_buffer);
  fwrite(bson_buffer, 1, bson_buffer_size, bson_file);
  memwriter_delete(memwriter);

  fclose(bson_file);

  bson_file = fopen(filepath, "rb");
  assert(bson_file != NULL);

  fseek(bson_file, 0L, SEEK_END);
  bson_file_size = ftell(bson_file);
  rewind(bson_file);

  bson_buffer = malloc(bson_file_size);
  assert(bson_buffer != NULL);
  bytes_read = fread(bson_buffer, 1, bson_file_size, bson_file);
  assert(bytes_read == bson_file_size);

  fclose(bson_file);

  read_args = grm_args_new();
  assert(read_args != NULL);

  error = frombson_read(read_args, bson_buffer);
  assert(error == ERROR_NONE);

  printf("\nParsed Argument container:\n");
  grm_dump(read_args, stdout);

  grm_args_delete(args);
}

int main(void)
{
  signal(SIGINT, signal_handler);
  signal(SIGQUIT, signal_handler);
  signal(SIGTERM, signal_handler);
  signal(SIGABRT, signal_handler);
  signal(SIGSEGV, signal_handler);
  atexit(cleanup);
  create_tmp_dir();
  assert(tmp_dir != NULL);

  test_bson();

  grm_finalize();

  return 0;
}
