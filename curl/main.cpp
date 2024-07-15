#include <curl/curl.h>
#include <string>
#include <cstdarg>

struct curl_slist *head_ = nullptr;
extern "C" void head_free__() {
	if(head_) {
		curl_slist_free_all(head_);
		head_ = nullptr;
	}
}
extern "C" CURLcode head__(CURL *curl, int argc, ...) {
	va_list argv;
	va_start(argv, argc);
	head_free__();
	for (int i = 0; i < argc; ++i) {
		const char*s = va_arg(argv, const char*);
		head_ = curl_slist_append(head_, s);
	}
	va_end(argv);
	CURLcode ret = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, head_);
	return ret;
}

class write___ {
public:
	std::string s_;
	FILE *fp_ = nullptr;
	int typ_ = 0;
};
static write___ write_;

size_t write__(void *ptr, size_t size, size_t nmemb, void *d0) {
	switch(write_.typ_) {
	case 0:
	case 2:
		write_.s_ += (const char*)ptr;
		switch(write_.typ_) {
		case 0:
			return size * nmemb;
		}
	case 1:
		return fwrite(ptr, size, nmemb, write_.fp_);
	}
	return 0;
}

extern "C" void write_close__() {
	switch(write_.typ_) {
	case 1:
	case 2:
		if(write_.fp_) {
			fclose(write_.fp_);
			write_.fp_ = nullptr;
		}
		break;
	}
	write_.s_.clear();
}
extern "C" CURLcode write_open__(CURL *curl, const char* s, int typ) {
	CURLcode ret = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write__);
	if(ret != CURLE_OK)
		return ret;
	write_close__();
	write_.typ_ = typ;
	switch(write_.typ_) {
	case 1:
	case 2:
		write_.fp_ = fopen(s, "wb");
		if(!write_.fp_)
			return CURLE_FILESIZE_EXCEEDED;
		break;
	}
	return curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
}
extern "C" const char* write_get__() {
	return write_.s_.c_str();
}
