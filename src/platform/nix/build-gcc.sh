set -e
g++ -std=c++11 -Os -s -fno-exceptions -fno-rtti -ffunction-sections -fdata-sections -Wl,-rpath=. -Wl,--gc-sections -Wno-invalid-source-encoding -DNDEBUG -D_POSIX_THREADS -D_POSIX_READER_WRITER_LOCKS main.cpp ../../libs/stb_vorbis/stb_vorbis.c ../../libs/minimp3/minimp3.cpp ../../libs/tinf/tinflate.c -I../../libs -I../../ -o../../../bin/OpenLara -L../../libs/libsm64/dist -lsm64 -lX11 -lGL -lm -lpthread -lSDL2 -lasound -lpulse-simple -lpulse
strip ../../../bin/OpenLara --strip-all --remove-section=.comment --remove-section=.note
