OUT := graphics

mode ?= debug

EXE := $(OUT).exe
RELEASE := $(OUT).zip

CURR_DIR := $(subst \,/,$(abspath .))/

SHD_DIR := $(CURR_DIR)shaders/
INC_DIR := $(CURR_DIR)include/
SRC_DIR := $(CURR_DIR)src/
OBJ_DIR := $(CURR_DIR)obj/$(mode)/

MODE_FILE := $(CURR_DIR).lastMode
LAST_MODE := $(strip $(shell if exist "$(MODE_FILE)" type "$(MODE_FILE)"))

JOBS := $(shell set /a NUMBER_OF_PROCESSORS-1)
ifeq ($(JOBS), 0)
	JOBS := 1
endif

CC := g++
FLAGS_OBJ := -std=c++17 -O2 -Werror -MMD -MP \
	-DVULKAN_HPP_NO_EXCEPTIONS \
	-DGLFW_INCLUDE_NONE \
	-DVULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1

INCLUDE := -I$(INC_DIR) -I$(subst \,/,$(VULKAN_SDK))/Include
FLAGS_EXE := -std=c++17 -O2 \
	-L$(subst \,/,$(VULKAN_SDK))/Lib \
	-static -lvulkan-1 -lglfw3 -lgdi32

SRCs_RAW := $(shell powershell -Command "Get-ChildItem -Path $(SRC_DIR) -Recurse -Filter *.cpp | ForEach-Object { $$_.FullName }")
SRCs := $(subst \,/,$(SRCs_RAW))
OBJs := $(patsubst $(SRC_DIR)%,$(OBJ_DIR)%,$(SRCs:.cpp=.o))
DEPs := $(OBJs:.o=.d)

SHADERS_RAW := $(shell powershell -Command "Get-ChildItem -File $(SHD_DIR) | ForEach-Object { $$_.FullName }")
SHADERS := $(subst \,/,$(SHADERS_RAW))

SPVs := $(addsuffix .spv,$(addprefix $(SHD_DIR)bin/,$(notdir $(SHADERS))))

ifeq ($(mode), debug)
	FLAGS_OBJ += -DDEBUG
else ifeq ($(mode), release)
	FLAGS_OBJ += -DRELEASE
	FLAGS_EXE += -mwindows
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

MAKE_NP := $(MAKE) --no-print-directory
PSFLAGS := -ExecutionPolicy Bypass

.PHONY: reload clean_run build run release 
.PHONY: clean clean_shaders clean_obj delete_exe
.PHONY: create delete

define CREATE_FILE
	$(eval EXT := $(1))
	$(eval DIR := $(2))
	$(eval NAME := $(3))

	@if exist "$(DIR)$(NAME).$(EXT)" (echo ERROR: $(DIR)$(NAME).$(EXT) already exists! & exit 1)
	@if not exist "$(DIR)$(dir $(NAME))" mkdir "$(DIR)$(dir $(NAME))"
	@powershell $(PSFLAGS) -File "$(CURR_DIR)AddFile.ps1" -FilePath "$(DIR)$(NAME)" -Type "$(EXT)"
	
	@echo $(NAME).$(EXT) created
endef

define DELETE_FILE
	$(eval EXT := $(1))
	$(eval DIR := $(2))
	$(eval NAME := $(3))

	@if exist "$(DIR)$(NAME).$(EXT)" powershell $(PSFLAGS) -Command "Remove-Item '$(DIR)$(NAME).$(EXT)'"
	@powershell $(PSFLAGS) -File "$(CURR_DIR)CleanUp.ps1" -path "$(DIR)$(dir $(NAME))" -root "$(DIR)"

	@echo $(NAME).$(EXT) deleted
endef

reload:
	@$(MAKE_NP) build mode=$(mode)
	@$(MAKE_NP) run

clean_run: 
	@$(MAKE_NP) clean_obj mode=$(mode)
	@$(MAKE_NP) clean_shaders
	@$(MAKE_NP) delete_exe

	@$(MAKE_NP) build mode=$(mode)
	@$(MAKE_NP) run

build: 
ifneq ($(LAST_MODE),$(mode))
	@if exist "$(CURR_DIR)$(EXE)" powershell $(PSFLAGS) -Command "Remove-Item '$(CURR_DIR)$(EXE)' -Force"
endif

	@$(MAKE_NP) -j$(JOBS) $(EXE) mode=$(mode)

	@echo $(mode) > "$(MODE_FILE)"

run:
	@if not exist $(EXE) (echo ERROR: $(EXE) not found & exit 1)

	@powershell $(PSFLAGS) -Command "Clear-Host"
	@$(CURR_DIR)$(EXE) 

release: 
	@$(MAKE_NP) clean_obj mode=release
	@$(MAKE_NP) clean_shaders

	@if exist "$(CURR_DIR)$(EXE)" ( \
		powershell $(PSFLAGS) -Command "Remove-Item '$(CURR_DIR)$(EXE)'" & \
		echo $(EXE) has been deleted \
	)

	@if exist "$(CURR_DIR)$(RELEASE)" ( \
		powershell $(PSFLAGS) -Command "Remove-Item '$(CURR_DIR)$(RELEASE)'" & \
		echo $(RELEASE) has been deleted \
	)

	@$(MAKE_NP) build mode=release

	@if exist "$(CURR_DIR)tmp" powershell $(PSFLAGS) -Command "Remove-Item '$(CURR_DIR)tmp' -Recurse -Force"
	@mkdir "$(CURR_DIR)tmp/shaders/bin/"

	@powershell $(PSFLAGS) -Command "Copy-Item -LiteralPath '$(CURR_DIR)$(EXE)' -Destination '$(CURR_DIR)tmp/'"
	@for %%F in ($(foreach spv,$(SPVs),"$(spv)")) do ( \
		powershell $(PSFLAGS) -Command "Copy-Item -LiteralPath '%%~F' -Destination '$(CURR_DIR)tmp/shaders/bin/'" || \
		exit 1 \
	)
	
	@powershell $(PSFLAGS) -Command "\
		$$ProgressPreference = 'SilentlyContinue'; \
		Compress-Archive -Path '$(CURR_DIR)tmp/*' -DestinationPath '$(CURR_DIR)$(RELEASE)' -Force; \
		Remove-Item '$(CURR_DIR)tmp' -Recurse -Force"

	@echo $(RELEASE) is ready to export

$(SHD_DIR)bin/%.spv: $(SHD_DIR)% 
	@if not exist "$(dir $@)" mkdir "$(dir $@)"
	glslc "$<" -o "$@"

$(EXE): $(SPVs) $(OBJs)
	$(CC) $(INCLUDE) $(OBJs) -o "$(CURR_DIR)$(EXE)" $(FLAGS_EXE) 

$(OBJ_DIR)%.o: $(SRC_DIR)%.cpp
	@if not exist "$(dir $@)" mkdir "$(dir $@)"
	$(CC) $(INCLUDE) $(FLAGS_OBJ) -c "$<" -o "$@"

clean:
	@$(MAKE_NP) clean_obj mode=debug
	@$(MAKE_NP) clean_obj mode=release
	@if exist "$(CURR_DIR)/obj" powershell $(PSFLAGS) -Command "Remove-Item '$(CURR_DIR)/obj' -Force"

	@$(MAKE_NP) clean_shaders

	@if exist "$(CURR_DIR)$(EXE)" ( \
		powershell $(PSFLAGS) -Command "Remove-Item '$(CURR_DIR)$(EXE)'" & \
		echo $(EXE) has been deleted \
	)

	@if exist "$(CURR_DIR)$(RELEASE)" ( \
		powershell $(PSFLAGS) -Command "Remove-Item '$(CURR_DIR)$(RELEASE)'" & \
		echo $(RELEASE) has been deleted \
	)

	@if exist "$(MODE_FILE)" powershell $(PSFLAGS) -Command "Remove-Item '$(MODE_FILE)'"

clean_obj:
	@if exist "$(OBJ_DIR)" powershell $(PSFLAGS) -Command "Remove-Item '$(OBJ_DIR)' -Recurse -Force"

	@echo $(mode) object files have been deleted

clean_shaders:
	@if exist "$(SHD_DIR)bin/" powershell $(PSFLAGS) -Command "Remove-Item '$(SHD_DIR)bin/' -Recurse -Force"

	@echo shaders have been deleted

delete_exe: 
	@if exist "$(CURR_DIR)$(EXE)" ( \
		powershell $(PSFLAGS) -Command "Remove-Item '$(CURR_DIR)$(EXE)'" & \
		echo $(EXE) has been deleted \
	)

create:
	@if not "$(word 4, $(MAKECMDGOALS))" == "" (echo ERROR: Too many arguments! & exit 1)
	@if "$(word 2, $(MAKECMDGOALS))" == "" (echo ERROR: File type not specified! & exit 1)
	@if "$(word 3, $(MAKECMDGOALS))" == "" (echo ERROR: File name not specified! & exit 1)

	$(eval TYPE := $(word 2, $(MAKECMDGOALS)))
	$(eval FILE := $(word 3, $(MAKECMDGOALS)))

	$(eval TARGET_EXT :=)
	$(if $(filter pair,$(TYPE)),$(eval TARGET_EXT := cpp hpp))
	$(if $(filter source,$(TYPE)),$(eval TARGET_EXT := cpp))
	$(if $(filter header,$(TYPE)),$(eval TARGET_EXT := hpp))

	@if "$(TARGET_EXT)" == "" (echo ERROR: Unknown option: $(TYPE) & exit 1)

	$(if $(filter hpp,$(TARGET_EXT)),$(call CREATE_FILE,hpp,$(INC_DIR),$(FILE)))
	$(if $(filter cpp,$(TARGET_EXT)),$(call CREATE_FILE,cpp,$(SRC_DIR),$(FILE)))
	
delete:
	@if not "$(word 4, $(MAKECMDGOALS))" == "" (echo ERROR: Too many arguments! & exit 1)
	@if "$(word 2, $(MAKECMDGOALS))" == "" (echo ERROR: File type not specified! & exit 1)
	@if "$(word 3, $(MAKECMDGOALS))" == "" (echo ERROR: File name not specified! & exit 1)

	$(eval TYPE := $(word 2, $(MAKECMDGOALS)))
	$(eval FILE := $(word 3, $(MAKECMDGOALS)))

	$(eval TARGET_EXT :=)
	$(if $(filter pair,$(TYPE)),$(eval TARGET_EXT := cpp hpp))
	$(if $(filter source,$(TYPE)),$(eval TARGET_EXT := cpp))
	$(if $(filter header,$(TYPE)),$(eval TARGET_EXT := hpp))

	@if "$(TARGET_EXT)" == "" (echo ERROR: Unknown option: $(TYPE) & exit 1)

	$(if $(filter hpp,$(TARGET_EXT)),$(call DELETE_FILE,hpp,$(INC_DIR),$(FILE)))
	$(if $(filter cpp,$(TARGET_EXT)),$(call DELETE_FILE,cpp,$(SRC_DIR),$(FILE)))

$(word 2, $(MAKECMDGOALS)) $(word 3, $(MAKECMDGOALS)):
	@:

-include $(DEPs)
