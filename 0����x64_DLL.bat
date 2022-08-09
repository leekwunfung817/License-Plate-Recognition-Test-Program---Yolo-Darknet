SET APP_NAME=CPosLicense

copy %APP_NAME%\video_sdk\x64\NetSdk.dll x64\Release\NetSdk.dll
copy %APP_NAME%\video_sdk\x64\H264Play.dll x64\Release\H264Play.dll
copy %APP_NAME%\video_sdk\x64\StreamReader.dll x64\Release\StreamReader.dll
copy %APP_NAME%\video_sdk\x64\ffmpeg.dll x64\Release\ffmpeg.dll


copy %APP_NAME%\video_sdk\x64\NetSdk.dll x64\Debug\NetSdk.dll
copy %APP_NAME%\video_sdk\x64\H264Play.dll x64\Debug\H264Play.dll
copy %APP_NAME%\video_sdk\x64\StreamReader.dll x64\Debug\StreamReader.dll
copy %APP_NAME%\video_sdk\x64\ffmpeg.dll x64\Debug\ffmpeg.dll
copy %APP_NAME%\darknet\pthreads\dll\x64\pthreadVC2.dll x64\Debug\pthreadVC2.dll
