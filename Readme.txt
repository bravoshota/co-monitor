Project Assessment:

CrossMonitor is a windows based client application for monitoring system resources. CrossMonitor application is under development and currently monitors  CPU, memory utilization and process count every minute and writes to the console output. How this information is transferred to server is out of scope of this project assessment and not implemented.

Project Notes:
  -  The current implementation has few bugs.
  -  Next user story is to include I/O read and write operations of the system to the console log. 
  
Tasks :
    1. Find and fix the bugs.
	1.1. FIXED CRASH: delete pimpl_.get(); was called in application destructor which was held in smart pointer container
	1.2. FIXED: on first record the cpu_usage was logged sometimes as 100%, sometimes as 0%, etc. The point was that CPU performance data query was immediately requested after initialize. So fixed it like: first log of cpu_usage is always 0, and on every next step the system has enough data to calculate a realistic value.
	1.3. FIXED: logs were not printed to console output after calling log::set_file(..). added missing add_console_log call in log::init().
	1.4. FIXED: added missing call for PdhCloseQuery on closing the app.
	1.5. FIXED: added missing CTRL cases in closing handler (BOOL WINAPI handler_helper(DWORD type))
	1.6. FIXED: changed deprecated array.as_string() to array.serialize()
	1.7. FIXED: fixed error when --logfile run option is indicated but empty value passed
	1.8. FIXED: added auto_flush=true to boost logger because there was no guarantee that app will log any record if either the terminal is closed or the app is crashed.
    2. Implement the new user story to include I/O read and write operation to the end of each entry and write unit tests.
	2.1. DONE: added I/O statistics output for every (active) volume in windows OS. first output record are zeroes and every next output is difference statistics between 2 neighbor logging periods.
	2.2. DONE: implemented UT CheckCollectData to check correctness of calculated collected data. it will check all performance values (including I/O operations)
    3. CrossMonitor.Client/application_client.cpp is implemented poorly. Refactor it to follow the best practices.
	3.1. moved remaining unnecessary implementation lines to "impl"
	3.2. added possibility for user to handle collectedData event. it is useful also for writing UT for it.
	3.3. to keep constructor signature I added default handler for this event. The default handler is used if the handler is not set during construction.

Prerequisites:
        Visual Studio 2015 for Windows.

How to run :
        Open the CrossMonitor.sln file in Visual Studio 2015.
        In Visual Studio IDE, change the architecture to x86.
        Build the project and run CrossMonitor.Client with appropriate arguments.
        

How to deliver :
This is how we are going to access and evaluate your submission, so please make sure to go through the following steps before submitting your answer.

1) Make sure to run unit tests, ensure that the application is compiling and all dependencies are included.
2) Delete the Debug and Packages folder to reduce the size of submission.
2) Zip the project and store/upload it to a shared location where Crossover team can access and download the project for evaluation, and add a link to the shared file in the answer field of this question.