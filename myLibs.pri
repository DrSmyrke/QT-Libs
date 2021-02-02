FILE = logger.cpp
!exists($$FILE):system(wget https://raw.githubusercontent.com/DrSmyrke/QT-Libs/master/$$FILE -O $$FILE)
SOURCES += $$FILE

FILE = logger.h
!exists($$FILE):system(wget https://raw.githubusercontent.com/DrSmyrke/QT-Libs/master/$$FILE -O $$FILE)
HEADERS += $$FILE



FILE = myfunctions.cpp
!exists($$FILE):system(wget https://raw.githubusercontent.com/DrSmyrke/QT-Libs/master/$$FILE -O $$FILE)
SOURCES += $$FILE

FILE = myfunctions.h
!exists($$FILE):system(wget https://raw.githubusercontent.com/DrSmyrke/QT-Libs/master/$$FILE -O $$FILE)
HEADERS += $$FILE



FILE = myproto.cpp
!exists($$FILE):system(wget https://raw.githubusercontent.com/DrSmyrke/QT-Libs/master/$$FILE -O $$FILE)
SOURCES += $$FILE

FILE = myproto.h
!exists($$FILE):system(wget https://raw.githubusercontent.com/DrSmyrke/QT-Libs/master/$$FILE -O $$FILE)
HEADERS += $$FILE

























# Need to discard STDERR so get path to NULL device
#win32 {
#    NULL_DEVICE = NUL # Windows doesn't have /dev/null but has NUL
#} else {
#    NULL_DEVICE = /dev/null
#}

#    ERASE_COMMAND = rm {myfunctions.cpp, myfunctions.h, logger.cpp, logger.h}
#win32|win64{
#    ERASE_COMMAND = del myfunctions.cpp, myfunctions.h, logger.cpp, logger.h
#}

#system($$ERASE_COMMAND 2> $$NULL_DEVICE)








#win32|win64{
#    system(powershell -Command "(New-Object Net.WebClient).DownloadFile('https://raw.githubusercontent.com/DrSmyrke/QT-Libs/master/logger.h', 'logger.h')")
#}else{
#    system(curl https://raw.githubusercontent.com/DrSmyrke/QT-Libs/master/logger.h > logger.h)
    
#    system(curl --proxy-negotiate -u: https://raw.githubusercontent.com/DrSmyrke/QT-Libs/master/myfunctions.cpp > myfunctions.cpp)
#}
