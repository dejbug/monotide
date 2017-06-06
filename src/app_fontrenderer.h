

struct FontRenderWorker
		: snippets::Worker
{
	struct Job
	{
		size_t index;
		size_t & count_rendered;

		Job(size_t, size_t &);
	};

	FontRenderWorker();
	virtual ~FontRenderWorker();

	void setup(HWND, window::BackgroundDC &,
		std::vector<font::EnumFontInfo> &);
	void queue(size_t, size_t &);
	char const * get_msg() const;

private:
	CRITICAL_SECTION mutex;
	HANDLE queue_event;
	HWND hwnd = nullptr;
	window::BackgroundDC * offscreen = nullptr;
	std::vector<font::EnumFontInfo> * fonts;
	std::vector<Job> jobs;
	char const * msg = nullptr;

	void task();
};
