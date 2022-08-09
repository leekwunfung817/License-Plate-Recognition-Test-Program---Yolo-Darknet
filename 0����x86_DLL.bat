SET APP_NAME=CPosLicense

copy %APP_NAME%\video_sdk\x86\NetSdk.dll Release\NetSdk.dll
copy %APP_NAME%\video_sdk\x86\H264Play.dll Release\H264Play.dll
copy %APP_NAME%\video_sdk\x86\StreamReader.dll Release\StreamReader.dll
copy %APP_NAME%\video_sdk\x86\ffmpeg.dll Release\ffmpeg.dll


copy %APP_NAME%\video_sdk\x86\NetSdk.dll Debug\NetSdk.dll
copy %APP_NAME%\video_sdk\x86\H264Play.dll Debug\H264Play.dll
copy %APP_NAME%\video_sdk\x86\StreamReader.dll Debug\StreamReader.dll
copy %APP_NAME%\video_sdk\x86\ffmpeg.dll Debug\ffmpeg.dll
copy %APP_NAME%\darknet\pthreads\dll\x86\pthreadVC2.dll Debug\pthreadVC2.dll
