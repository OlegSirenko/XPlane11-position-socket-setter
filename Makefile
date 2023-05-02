BUILDDIR	:=	./build
SRC_BASE	:=	.
TARGET		:= Position
SOURCES = \
	position_receiver.cpp
LIBS =
INCLUDES = \
	-I$(SRC_BASE)/SDK/CHeaders/XPLM \
	-I$(SRC_BASE)/SDK/CHeaders/Widgets

DEFINES = -DXPLM200=1 -DXPLM210=1 -DAPL=0 -DIBM=0 -DLIN=1
VPATH = $(SRC_BASE)
CSOURCES	:= $(filter %.c, $(SOURCES))
CXXSOURCES	:= $(filter %.cpp, $(SOURCES))
CDEPS64			:= $(patsubst %.c, $(BUILDDIR)/obj64/%.cdep, $(CSOURCES))
CXXDEPS64		:= $(patsubst %.cpp, $(BUILDDIR)/obj64/%.cppdep, $(CXXSOURCES))
COBJECTS64		:= $(patsubst %.c, $(BUILDDIR)/obj64/%.o, $(CSOURCES))
CXXOBJECTS64	:= $(patsubst %.cpp, $(BUILDDIR)/obj64/%.o, $(CXXSOURCES))
ALL_DEPS64		:= $(sort $(CDEPS64) $(CXXDEPS64))
ALL_OBJECTS64	:= $(sort $(COBJECTS64) $(CXXOBJECTS64))
CFLAGS := $(DEFINES) $(INCLUDES) -fPIC -fvisibility=hidden

.PHONY: all clean $(TARGET)

.SECONDARY: $(ALL_OBJECTS) $(ALL_OBJECTS64) $(ALL_DEPS)
# Target rules - these just induce the right .xpl files.
$(TARGET): $(BUILDDIR)/$(TARGET)/64/position_plugin.xpl

$(BUILDDIR)/$(TARGET)/64/position_plugin.xpl: $(ALL_OBJECTS64)
	@echo Linking $@
	mkdir -p $(dir $@)
	gcc -m64 -static-libgcc -shared -Wl,--version-script=exports.txt -o $@ $(ALL_OBJECTS64) $(LIBS)

$(BUILDDIR)/obj64/%.o : %.c
	mkdir -p $(dir $@)
	g++ $(CFLAGS) -m64 -c $< -o $@
	g++ $(CFLAGS) -MM -MT $@ -o $(@:.o=.cdep) $<

$(BUILDDIR)/obj64/%.o : %.cpp
	mkdir -p $(dir $@)
	g++ $(CFLAGS) -m64 -c $< -o $@
	g++ $(CFLAGS) -MM -MT $@ -o $(@:.o=.cppdep) $<

clean:
	@echo Cleaning out everything.
	rm -rf $(BUILDDIR)


-include $(ALL_DEPS64)


