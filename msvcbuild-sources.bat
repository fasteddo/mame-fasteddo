@rem old
@rem make -j11 MSVC_BUILD=1 DEBUG=1 SYMBOLS=1 NOWERROR=1 DIRECTINPUT=8 DONT_USE_NETWORK=1 MINGW64=c:\msys64\mingw64

@rem http://wiki.mamedev.org/index.php?title=Building_MAME_using_Microsoft_Visual_Studio_compilers
@rem https://docs.mamedev.org/initialsetup/compilingmame.html

@rem make SUBTARGET=tiny vs2015 
@rem make vs2015 SUBTARGET=drivername SOURCES=src\mame\drivers\drivername.cpp
make vs2019 REGENIE=1 MODERN_WIN_API=1 NO_USE_PORTAUDIO=1 ^
PYTHON_EXECUTABLE=d:/msys64-2022-01-12/mingw64/bin/python.exe ^
SOURCES=^
src/mame/capcom/1942.cpp,^
src/mame/midw8080/8080bw.cpp,^
src/mame/atari/asteroid.cpp,^
src/mame/atari/atarisy2.cpp,^
src/mame/stern/berzerk.cpp,^
src/mame/dataeast/btime.cpp,^
src/mame/atari/bzone.cpp,^
src/mame/atari/centiped.cpp,^
src/mame/nichibutsu/cclimber.cpp,^
src/mame/nintendo/dkong.cpp,^
src/mame/galaxian/fastfred.cpp,^
src/mame/namco/galaga.cpp,^
src/mame/galaxian/galaxian.cpp,^
src/mame/gottlieb/gottlieb.cpp,^
src/mame/irem/m52.cpp,^
src/mame/namco/mappy.cpp,^
src/mame/midway/mcr.cpp,^
src/mame/atari/missile.cpp,^
src/mame/midw8080/mw8080bw.cpp,^
src/mame/pacman/pacman.cpp,^
src/mame/pacman/pengo.cpp,^
src/mame/namco/polepos.cpp,^
src/mame/atari/pong.cpp,^
src/mame/nintendo/popeye.cpp,^
src/mame/namco/rallyx.cpp,^
src/mame/snk/snk6502.cpp,^
src/mame/sega/system1.cpp,^
src/mame/taito/taitosj.cpp,^
src/mame/atari/tempest.cpp,^
src/mame/sega/turbo.cpp,^
src/mame/midway/williams.cpp,^
src/mame/sega/zaxxon.cpp
