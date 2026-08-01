OUT := graphics.exe

CURR_DIR := $(subst \,/,$(abspath .))/

SHD_DIR := $(CURR_DIR)shaders/
INC_DIR := $(CURR_DIR)include/
SRC_DIR := $(CURR_DIR)src/
OBJ_DIR := $(CURR_DIR)obj/

JOBS := $(shell set /a NUMBER_OF_PROCESSORS-1)

CC := g++
FLAGS_OBJ := -std=c++17 -O2 -Werror \
	-DVULKAN_HPP_NO_EXCEPTIONS \
	-DGLFW_INCLUDE_VULKAN \
	-DVULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1

INCLUDE := -I$(INC_DIR) -I$(subst \,/,$(VULKAN_SDK))/Include
FLAGS_EXE := -std=c++17 -O2 \
	-L$(subst \,/,$(VULKAN_SDK))/Lib \
	-lvulkan-1 -lglfw3 \

PSFLAGS := -ExecutionPolicy Bypass

SRCs_RAW := $(shell powershell -Command "Get-ChildItem -Recurse -Filter *.cpp | ForEach-Object { $$_.FullName }")
SRCs := $(subst \,/,$(SRCs_RAW))
OBJs := $(patsubst $(SRC_DIR)%,$(OBJ_DIR)%,$(SRCs:.cpp=.o))

SHADERS_RAW := $(shell powershell -Command "Get-ChildItem -File $(SHD_DIR) | ForEach-Object { $$_.FullName }")
SHADERS := $(subst \,/,$(SHADERS_RAW))

SPV_FILES := $(addsuffix .spv,$(addprefix $(SHD_DIR)bin/,$(notdir $(SHADERS))))

mode ?= debug
ifeq ($(mode), debug)
    FLAGS_OBJ += -DDEBUG
else ifeq ($(mode), release)
    FLAGS_OBJ += -DRELEASE
else
    $(error Unknown build mode)
endif

# --- Sanity checks ---

GPP_PATH := $(shell where g++ 2>NUL)
PRIMARY_GPP := $(firstword $(GPP_PATH))

ifeq ($(strip $(PRIMARY_GPP)),)
  $(error No g++ found in PATH)
endif

ifndef VULKAN_SDK
  $(error VULKAN_SDK is not set)
endif

ifeq ($(wildcard $(VULKAN_SDK)/Include/vulkan/vulkan.hpp),)
  $(error Vulkan header (vulkan.hpp) not found)
endif

ifeq ($(wildcard $(VULKAN_SDK)/Lib/vulkan-1.lib),)
  $(error Vulkan library (vulkan-1.lib) not found)
endif

# --- End sanity checks ---

.PHONY: reload restart all clean_run run build build_shaders clean clean_shaders clean_obj create delete

reload: 
	@make --no-print-directory -j$(JOBS) all
	@make --no-print-directory run

restart: clean_shaders
	@make --no-print-directory -j$(JOBS) all
	@make --no-print-directory run

clean_run: clean
	@make --no-print-directory -j$(JOBS) all
	@make --no-print-directory run

build: $(OBJs)

build_shaders: $(SPV_FILES)

$(SHD_DIR)bin/%.spv: $(SHD_DIR)% 
	@if not exist "$(dir $@)" mkdir "$(dir $@)"
	glslc "$<" -o "$@"

run:
	@if not exist "$(CURR_DIR)$(OUT)" (make --no-print-directory -j$(NUMBER_OF_PROCESSORS) all)

	@powershell -Command "Clear-Host"
	@$(OUT) 

all: build_shaders $(OBJs)
	$(CC) $(INCLUDE) $(OBJs) -o "$(CURR_DIR)$(OUT)" $(FLAGS_EXE) 

$(OBJ_DIR)%.o: $(SRC_DIR)%.cpp
	@if not exist "$(dir $@)" mkdir "$(dir $@)"
	$(CC) $(INCLUDE) $(FLAGS_OBJ) -c "$<" -o "$@"

clean_obj:
	@if exist "$(OBJ_DIR)" powershell $(PSFLAGS) -Command "Remove-Item '$(OBJ_DIR)' -Recurse -Force"

	@echo object files have been deleted

clean_shaders:
	@if exist "$(SHD_DIR)bin/" powershell $(PSFLAGS) -Command "Remove-Item '$(SHD_DIR)bin/' -Recurse -Force"

	@echo shaders have been deleted

clean:
	@make --no-print-directory clean_obj
	@make --no-print-directory clean_shaders

	@if exist "$(CURR_DIR)$(OUT)" powershell $(PSFLAGS) -Command "Remove-Item '$(CURR_DIR)$(OUT)'"
	
	@echo $(OUT) has been deleted

create:
	@if not "$(word 3, $(MAKECMDGOALS))" == "" (echo ERROR: Too many arguments! & exit 1)
	@if "$(word 2, $(MAKECMDGOALS))" == "" (echo ERROR: File name not specified! & exit 1)

	$(eval FILE := $(word 2, $(MAKECMDGOALS)))
	$(eval DIRPATH := $(dir $(FILE)))

	@if exist "$(INC_DIR)$(FILE).hpp" (echo ERROR: include/$(FILE).hpp already exists! & exit 1)
	@if exist "$(SRC_DIR)$(FILE).cpp" (echo ERROR: src/$(FILE).cpp already exists! & exit 1)

	@if not exist "$(INC_DIR)$(DIRPATH)" mkdir "$(INC_DIR)$(DIRPATH)"
	@if not exist "$(SRC_DIR)$(DIRPATH)" mkdir "$(SRC_DIR)$(DIRPATH)"

	@powershell $(PSFLAGS) -File "$(CURR_DIR)AddFile.ps1" -FilePath "$(INC_DIR)$(FILE)" -Type "hpp"
	@powershell $(PSFLAGS) -File "$(CURR_DIR)AddFile.ps1" -FilePath "$(SRC_DIR)$(FILE)" -Type "cpp"

	@echo $(FILE).cpp and $(FILE).hpp created

delete:
	@if not "$(word 3, $(MAKECMDGOALS))" == "" (echo ERROR: Too many arguments! & exit 1)
	@if "$(word 2, $(MAKECMDGOALS))" == "" (echo ERROR: File name not specified! & exit 1)

	$(eval FILE := $(word 2, $(MAKECMDGOALS)))

	@if exist "$(INC_DIR)$(FILE).hpp" powershell $(PSFLAGS) -Command "Remove-Item '$(INC_DIR)$(FILE).hpp'"
	@if exist "$(SRC_DIR)$(FILE).cpp" powershell $(PSFLAGS) -Command "Remove-Item '$(SRC_DIR)$(FILE).cpp'"

	@powershell $(PSFLAGS) -File "$(CURR_DIR)CleanUp.ps1" -path "$(INC_DIR)$(dir $(FILE))" -root "$(INC_DIR)"
	@powershell $(PSFLAGS) -File "$(CURR_DIR)CleanUp.ps1" -path "$(SRC_DIR)$(dir $(FILE))" -root "$(SRC_DIR)"

	@echo $(FILE).cpp and $(FILE).hpp deleted

$(word 2, $(MAKECMDGOALS)):
	@:
