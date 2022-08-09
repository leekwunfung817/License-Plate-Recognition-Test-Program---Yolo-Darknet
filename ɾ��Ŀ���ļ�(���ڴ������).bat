SET APP_NAME=CPosLicense

del /Q /S %APP_NAME%\Debug\*.*
del /Q /S %APP_NAME%\Release\*.*
del /Q /S Debug\*.ilk
del /Q /S Debug\*.pdb
del /Q /S Debug\%APP_NAME%.exe
del /Q /S Debug\NetSdk.dll
del /Q /S Debug\H264Play.dll
del /Q /S Debug\StreamReader.dll
del /Q /S Debug\pthreadVC2.dll
rd /Q /S Debug\cpos_ctr
rd /Q /S Debug\cpos_lpr
rd /Q /S Debug\cpos_ocr

del /Q /S Release\*.pdb
del /Q /S Release\*.ipdb
del /Q /S Release\*.iobj
del /Q /S Release\NetSdk.dll
del /Q /S Release\H264Play.dll
del /Q /S Release\StreamReader.dll
rd /Q /S Release\cpos_ctr
rd /Q /S Release\cpos_lpr
rd /Q /S Release\cpos_ocr

del /Q *.VC.db
del /Q *.sdf
rd /Q /S ipch

del /Q /S %APP_NAME%\x64\Debug\*.*
del /Q /S %APP_NAME%\x64\Release\*.*
del /Q /S x64\Debug\*.ilk
del /Q /S x64\Debug\*.pdb
del /Q /S x64\Debug\%APP_NAME%.exe
del /Q /S x64\Debug\NetSdk.dll
del /Q /S x64\Debug\H264Play.dll
del /Q /S x64\Debug\StreamReader.dll
del /Q /S x64\Debug\ffmpeg.dll
rd /Q /S x64\Debug\cpos_ctr
rd /Q /S x64\Debug\cpos_lpr
rd /Q /S x64\Debug\cpos_ocr


del /Q /S x64\Release\*.pdb
del /Q /S x64\Release\*.ipdb
del /Q /S x64\Release\*.iobj
del /Q /S x64\Release\NetSdk.dll
del /Q /S x64\Release\H264Play.dll
del /Q /S x64\Release\StreamReader.dll
del /Q /S x64\Release\ffmpeg.dll
rd /Q /S x64\Release\cpos_ctr
rd /Q /S x64\Release\cpos_lpr
rd /Q /S x64\Release\cpos_ocr

