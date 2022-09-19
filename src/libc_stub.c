// void _init(void) {}
// void _fini(void) {}
int __errno;
// void *__dso_handle = (void *)&__dso_handle;

void _exit(int x) {
	while (1)
		;
}
void _kill(int x) {
}
int _getpid() {
	return -1;
}
void _write(char x) {
}
void _close() {
}
void _fstat() {
}
void _isatty() {
}
void _lseek() {
}
void _read() {
}
