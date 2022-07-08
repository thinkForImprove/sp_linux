#ifdef __cplusplus
extern "C"
{
#endif

int read_profile_string(const char *section, const char *key, char *value, int size, const char *default_value, const char *file);
int read_profile_int(const char *section, const char *key, int default_value, const char *file);
int write_profile_string(const char *section, const char *key, const char *value, const char *file);
int read_all_section(char (*ary)[100], int &count, const char *file);
int read_every_section_KeyValue_data(const char *section, char *buffer, int size, const char *file);

#ifdef __cplusplus
}
#endif
