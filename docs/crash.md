# Crash trace

```sh
${VCPKG_ROOT}/installed/x64-linux/tools/breakpad/dump_syms ./your_application > your_application.sym

head -n 1 your_application.sym
# Output will look like: MODULE Linux x86_64 26BF611B7ACC305A9FC5C535A513256F0 your_application

mkdir -p ./symbols/your_application/26BF611B7ACC305A9FC5C535A513256F0/

mv your_application.sym ./symbols/your_application/26BF611B7ACC305A9FC5C535A513256F0/

./minidump_stackwalk crash_file.dmp ./symbols > decoded_stacktrace.txt
```