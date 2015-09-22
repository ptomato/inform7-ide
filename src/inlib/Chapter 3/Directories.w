[Directories::] Directories.

@Purpose: Scanning directories on the host filing system.

@p Abstraction.
All of this abstracts the code already found in the platform definitions.

@c
typedef struct scan_directory {
	void *directory_handle;
	char directory_name_written_out[MAX_FILENAME_LENGTH];
	MEMORY_MANAGEMENT
} scan_directory;

@

@c
scan_directory *Directories::open(pathname *P) {
	scan_directory *D = CREATE(scan_directory);
	Pathnames::to_string(D->directory_name_written_out, P);
	D->directory_handle = Platform::opendir(D->directory_name_written_out);
	return D;
}

int Directories::next(scan_directory *D, char *leafname) {
	return Platform::readdir(D->directory_handle, D->directory_name_written_out, leafname);
}

void Directories::close(scan_directory *D) {
	Platform::closedir(D->directory_handle);
}
