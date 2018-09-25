Project Assessment:

CrossMonitor is a windows based client application for monitoring system resources. CrossMonitor application is under development and currently monitors  CPU, memory utilization and process count every minute and writes to the console output. How this information is transferred to server is out of scope of this project assessment and not implemented.

Project Notes:
  -  The current implementation has few bugs.
  -  Next user story is to include I/O read and write operations of the system to the console log. 
  
Tasks :
    1. Find and fix the bugs.
    2. Implement the new user story to include I/O read and write operation to the end of each entry and write unit tests.
    3. CrossMonitor.Client/application_client.cpp is implemented poorly. Refactor it to follow the best practices.

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