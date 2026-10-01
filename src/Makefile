#************************************************
#*                                              *
#*   TP 1&2    (c) 2017 J. FABRIZIO             *
#*                                              *
#*                               LRDE EPITA     *
#*                                              *
#************************************************

CC = g++

CPP_FILES = objects/sphere.cc \
			objects/triangle.cc \
			objects/bvh.cc \
			objects/mesh.cc \
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

OBJ_FILES = $(CPP_FILES:.cc=.o)
EXEC = ../main

CPPFLAGS += -I. -I../includes
CXXFLAGS += -Wall -Wextra -O3 -g -std=c++20 -march=native
LDLIBS += -lm

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
	@make --no-print-directory main-build -Iincludes ; \
	sta=$$?;          \
	$(call color,4); \
	echo "*********** End Compilation **************"; \
	$(call default_color); \
	exit $$sta;

main-build: pre-build build

build: $(OBJ_FILES)
	$(CC) -o $(EXEC) $(OBJ_FILES) $(CXX_FLAGS) $(LDXX_FLAGS) $(LDLIBS)

clean:
	rm -f $(EXEC)
	rm -f $(OBJ_FILES)
