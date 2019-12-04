# Get a good guess as to our compile target
gcc_machine := $(subst -, ,$(shell $(CC) -dumpmachine))

GCC_ARCH := $(word 1,$(gcc_machine))
OS1 := $(word 2,$(gcc_machine))
OS2 := $(word 3,$(gcc_machine))

ifeq "$(ARCH)" ""
  ifeq "$(GCC_ARCH)" ""
  $(error "Unable to determine architecture")
  endif
endif

ifeq "$(OS1)" ""
  ifeq "$(OS2)" ""
    $(error "Unable to determine operating system")
  endif
endif

# Consolidate the GCC architecture value.
# If it was user defined, and it disagrees with the GCC value, we are probably
# trying a multilib build
ifdef ARCH
  # user arch and GCC arch did not match
  ifneq "$(ARCH)" "$(GCC_ARCH)"
    ifeq ($(findstring $(ARCH), i686 x86_64),)
    $(error "Unsupported architecture $(ARCH)")
    endif

    ifeq "$(ARCH)" "i686"
      CFLAGS := $(CFLAGS) -m32
      CXXFLAGS := $(CXXFLAGS) -m32
      LDFLAGS := $(LDFLAGS) -m32
    else ifeq "$(ARCH)" "x86_64")
      CFLAGS := $(CFLAGS) -m64
      CXXFLAGS := $(CXXFLAGS) -m64
      LDFLAGS := $(LDFLAGS) -m64
    endif
  endif
else
  ifeq ($(findstring $(GCC_ARCH), i686 x86_64),)
  $(error "Unsupported architecture $(ARCH)")
  endif

  ARCH := $(GCC_ARCH)
endif

ifeq ($(findstring $(OS1), linux cygwin mingw32 apple),)
  ifeq ($(findstring $(OS2), linux cygwin mingw32 apple),)
    $(error "Unsupported operating system $(OS1), $(OS2)")
  else
    OS := $(OS2)
  endif
else
  OS := $(OS1)
endif

# Output artifact functions
ifeq "$(OS)" "mingw32"
  lib-name = $(1).dll
  exe-name = $(1).exe
else ifeq "$(OS)" "cygwin"
  lib-name = $(1).dll
  exe-name = $(1).exe
else ifeq "$(OS)" "apple"
  lib-name = lib$(1).dylib
  exe-name = $(1)
else
  lib-name = lib$(1).so
  exe-name = $(1)
endif

export OS
export ARCH
export CFLAGS
export CXXFLAGS
export LDFLAGS

$(info === Build configuration ===)
$(info Archtecture: $(ARCH))
$(info Operating system: $(OS))
