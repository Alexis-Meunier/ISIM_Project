#************************************************
#*                                              *
#*   TP 1&2    (c) 2017 J. FABRIZIO             *
#*                                              *
#*                               LRDE EPITA     *
#*                                              *
#************************************************

CC = g++

SKEL_CPP_FILES = objects/sphere.cc \
				 objects/triangle.cc \
				 objects/bvh.cc \
				 objects/mesh.cc \
				 objects/blob.cc \
				 moteur.cc \
				 perlin3D.cc \
				 objects/blob.cc \
				 camera/camera.cc \
				 image/image.cc \
				 light/circle_light.cc \
				 light/point_light.cc \
				 texture/ImageTexture.cc \
				 texture/UniformTexture.cc \
				 texture/LightTexture.cc \
				 texture/ProceduralTexture.cc \
				 utils/color.cc \
				 utils/scene.cc \
				 utils/vector4.cc \
				 utils/point4.cc

SKEL_HH_FILES  = point3.hh

CPP_FILES =  $(SKEL_CPP_FILES)
H_FILES =  $(SKEL_HH_FILES)
OBJ_FILES = $(CPP_FILES:.cc=.o)
EXEC = main

CXX_FLAGS += -Wall -Wextra -O3 -g -std=c++20
CXX_FLAGS += -lm
CXX_FLAGS += -march=native
#CXX_FLAGS += -fopt-info-vec-optimized -fopt-info-vec-missed -ftree-vectorize
LDXX_FLAGS =

SKEL_DIST_DIR = tifo_skel_tp

#For gcc 4.9
#CXXFLAGS+=-fdiagnostics-color=auto
export GCC_COLORS=1

define color
    if test -n "${TERM}" ; then\
        if test `tput colors` -gt 0 ; then \
            tput setaf $(1); \
        fi;\
    fi
endef

define default_color
    if test -n "${TERM}" ; then\
        if test `tput colors` -gt 0 ; then  tput sgr0 ; fi; \
    fi
endef


all: post-build

pre-build:
	@$(call color,4)
	@echo "******** Starting Compilation ************"
	@$(call default_color)

post-build:
	@make --no-print-directory main-build ; \
	sta=$$?;          \
	$(call color,4); \
	echo "*********** End Compilation **************"; \
	$(call default_color); \
	exit $$sta;

main-build: pre-build build

build: $(OBJ_FILES)
	$(CC) -o $(EXEC) $(OBJ_FILES) $(CXX_FLAGS) $(LDXX_FLAGS)
	#$(CC) tp2.cpp -o tp2 $(OBJ_FILES) $(CXX_FLAGS) $(LDXX_FLAGS)

clean:
	rm -f $(EXEC)
	rm -f $(OBJ_FILES)
	rm -f $(SKEL_DIST_DIR)
	rm -f $(SKEL_DIST_DIR).tar.bz2