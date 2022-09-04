# AsioNetworking

Client/server application based on the asio C++ library. The development was done in a Docker container.

The ASIO library is moved to /usr/include when connecting to the container so the user can include the library with "<>".
Due to a compatiblity issue between Windows and Linux, the install-gtest script has "/r" appended to every line and that makes the script useless.
  To solve that: copy the text from it, delete it's content and echo it back to the file.
  After that GoogleTest will be ready to use.
 
 Using CMake 2 tests were made.
