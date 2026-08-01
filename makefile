OUT := GaussianSplatViewer

mode ?= debug

EXE := $(OUT).exe
RELEASE := $(OUT).zip

CURR_DIR := $(subst \,/,$(abspath .))/

SHD_DIR := shaders/
INC_DIR := include/
SRC_DIR := source/
RES_DIR := resource/
GPU_DIR := gpu/
OBJ_DIR := obj/$(mode)/

SHD_INC_DIR := $(SHD_DIR)include/
GEN_DIR := $(INC_DIR)generated/

SCRP_DIR := scripts/

MODE_FILE := .lastMode
LAST_MODE := $(strip $(shell if exist "$(MODE_FILE)" type "$(MODE_FILE)"))

JOBS := $(shell set /a NUMBER_OF_PROCESSORS-1)
ifeq ($(JOBS), 0)
	JOBS := 1
endif

CC := g++
FLAGS_OBJ := -std=c++17 -O2 -MMD -MP \
	-Werror \
	-DGLFW_INCLUDE_NONE \
	-DVULKAN_HPP_NO_EXCEPTIONS -DVULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1 \
	-DWIN32_LEAN_AND_MEAN -D_WIN32_WINNT=0x0A00 -DWINVER=0x0A00 \

# for future #
# -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Wshadow 
# 
# threading tests
# memory tests

GLSLC = glslc
FLAGS_SPV = -MD -I"$(SHD_INC_DIR)"

WINDRES := windres

INCLUDE := -I"$(INC_DIR)" -I"$(subst \,/,$(VULKAN_SDK))/Include"
FLAGS_EXE := -std=c++17 -O2 \
	-L"$(subst \,/,$(VULKAN_SDK))/Lib/" \
	-static -lvulkan-1 -lglfw3 -lgdi32 -lole32 -luuid -ldwmapi

SRCs_RAW := $(shell \
	powershell -Command "$$root = (Get-Location).Path; \
	Get-ChildItem -LiteralPath '$(SRC_DIR)' -Recurse -File -Filter '*.cpp' \
	| ForEach-Object { $$_.FullName.Substring($$root.Length + 1) }")

SHADERS_RAW := $(shell \
	powershell -Command "$$root = (Get-Location).Path; \
	Get-ChildItem -LiteralPath '$(SHD_DIR)' -Recurse -File \
	| ForEach-Object { \
		$$relativePath = ($$_.FullName.Substring($$root.Length + 1) -replace '\\','/'); \
		if($$relativePath -notlike '$(SHD_INC_DIR)*') { $$relativePath } \
	}")

SRCs := $(subst \,/,$(SRCs_RAW))
SHADERS := $(subst \,/,$(SHADERS_RAW))

OBJs := $(patsubst $(SRC_DIR)%,$(OBJ_DIR)%,$(SRCs:.cpp=.o))

SHADER_PATHS := $(patsubst $(SHD_DIR)%,%,$(SHADERS))

SPVs := $(addprefix $(GPU_DIR),$(addsuffix .spv,$(SHADER_PATHS)))
GENERATED_SHADERS := $(addprefix $(GEN_DIR),$(addsuffix .spv.hpp,$(SHADER_PATHS)))

RESOURCE_OBJ := $(OBJ_DIR)$(OUT).o

OBJ_DEPs := $(OBJs:.o=.d)
SPV_DEPs := $(addsuffix .d,$(SPVs))
GENERATED_SHADER_DEPS := $(addsuffix .d,$(GENERATED_SHADERS))

ifeq ($(mode), debug)
	FLAGS_OBJ += -DDEBUG

	SHADER_OUTPUTS := $(SPVs)
	SHADER_DEPS := $(SPV_DEPS)
else ifeq ($(mode), release)
	FLAGS_OBJ += -DRELEASE -DNDEBUG
	FLAGS_EXE += -mwindows

	SHADER_OUTPUTS := $(GENERATED_SHADERS)
	SHADER_DEPS := $(GENERATED_SHADER_DEPS)
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

VULKAN_HEADER_FOUND := $(strip \
	$(shell if exist "$(VULKAN_SDK)\Include\vulkan\vulkan.hpp" echo 1) \
)

VULKAN_LIBRARY_FOUND := $(strip \
	$(shell if exist "$(VULKAN_SDK)\Lib\vulkan-1.lib" echo 1) \
)

ifeq ($(VULKAN_HEADER_FOUND),)
	$(error Vulkan header (vulkan.hpp) not found)
endif

ifeq ($(VULKAN_LIBRARY_FOUND),)
	$(error Vulkan library (vulkan-1.lib) not found)
endif

# --- End sanity checks ---

MAKE_NP := $(MAKE) --no-print-directory
PSFLAGS := -ExecutionPolicy Bypass

.PHONY: reload clean_run build run release 
.PHONY: clean clean_shaders clean_obj delete_exe
.PHONY: create delete create_shader delete_shader

define CREATE_DIR
	$(eval DIR := $(dir $(1)))

	@mkdir "$(DIR)" 2>NUL || \
		if not exist "$(DIR)" exit 1
endef

define CREATE_CODE
	$(eval EXT := $(1))
	$(eval DIR := $(2))
	$(eval NAME := $(3))

	@if exist "$(DIR)$(NAME).$(EXT)" (echo ERROR: $(DIR)$(NAME).$(EXT) already exists! & exit 1)
	@if not exist "$(DIR)$(dir $(NAME))" mkdir "$(DIR)$(dir $(NAME))"
	@powershell $(PSFLAGS) -File "$(SCRP_DIR)AddFile.ps1" -FilePath "$(DIR)$(NAME)" -Type "$(EXT)"
	
	@echo $(NAME).$(EXT) created
endef

define CREATE_SHADER
	$(eval TYPE := $(1))
	$(eval DIR := $(2))
	$(eval NAME := $(3))

	@if exist "$(DIR)$(NAME)" (echo ERROR: $(DIR)$(NAME) already exists! & exit 1)
	@if not exist "$(DIR)$(dir $(NAME))" mkdir "$(DIR)$(dir $(NAME))"
	@powershell $(PSFLAGS) -File "$(SCRP_DIR)AddShader.ps1" -FilePath "$(DIR)$(NAME)" -Type "$(TYPE)"

	@echo $(NAME) created
endef

define DELETE_FILE
	$(eval EXT := $(1))
	$(eval DIR := $(2))
	$(eval NAME := $(3))

	@if exist "$(DIR)$(NAME).$(EXT)" powershell $(PSFLAGS) -Command "Remove-Item '$(DIR)$(NAME).$(EXT)'"
	@powershell $(PSFLAGS) -File "$(SCRP_DIR)CleanUp.ps1" -path "$(DIR)$(dir $(NAME))" -root "$(DIR)"

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
	@if not exist "$(CURR_DIR)$(EXE)" (echo ERROR: $(EXE) not found & exit 1)

	@powershell $(PSFLAGS) -Command "Clear-Host"
	@"$(CURR_DIR)$(EXE)"

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

	@powershell $(PSFLAGS) -Command " \
		$$ProgressPreference = 'SilentlyContinue'; \
		Compress-Archive -LiteralPath '$(CURR_DIR)$(EXE)' -DestinationPath '$(CURR_DIR)$(RELEASE)' -Force"

	@echo $(RELEASE) is ready to export

clean:
	@$(MAKE_NP) clean_obj mode=debug
	@$(MAKE_NP) clean_obj mode=release
	@if exist "$(CURR_DIR)obj" powershell $(PSFLAGS) -Command "Remove-Item '$(CURR_DIR)obj' -Force"

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
	@if exist "$(GPU_DIR)" powershell $(PSFLAGS) -Command "Remove-Item '$(GPU_DIR)' -Recurse -Force"
	@if exist "$(GEN_DIR)" powershell $(PSFLAGS) -Command "Remove-Item '$(GEN_DIR)' -Recurse -Force"

	@echo shader binaries have been deleted

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

	$(if $(filter hpp,$(TARGET_EXT)),$(call CREATE_CODE,hpp,$(INC_DIR),$(FILE)))
	$(if $(filter cpp,$(TARGET_EXT)),$(call CREATE_CODE,cpp,$(SRC_DIR),$(FILE)))
	
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

create_shader:
	@if not "$(word 4, $(MAKECMDGOALS))" == "" (echo ERROR: Too many arguments! & exit 1)
	@if "$(word 2, $(MAKECMDGOALS))" == "" (echo ERROR: Shader type not specified! & exit 1)
	@if "$(word 3, $(MAKECMDGOALS))" == "" (echo ERROR: Shader name not specified! & exit 1)

	$(eval TYPE := $(word 2, $(MAKECMDGOALS)))
	$(eval FILE := $(word 3, $(MAKECMDGOALS)))
	$(eval NORMALIZED_FILE := $(subst \,/,$(FILE)))

	@if "$(TYPE)" == "source" if not "$(filter include/%,$(NORMALIZED_FILE))" == "" ( \
		echo ERROR: Shader sources cannot be created inside shaders/include/ & exit 1 \
	)

	$(eval TARGET_DIR :=)
	$(if $(filter source,$(TYPE)),$(eval TARGET_DIR := $(SHD_DIR)))
	$(if $(filter header,$(TYPE)),$(eval TARGET_DIR := $(SHD_INC_DIR)))

	@if "$(TARGET_DIR)" == "" (echo ERROR: Unknown option: $(TYPE) & exit 1)

	$(call CREATE_SHADER,$(TYPE),$(TARGET_DIR),$(FILE))

delete_shader:
	@if not "$(word 3, $(MAKECMDGOALS))" == "" (echo ERROR: Too many arguments! & exit 1)
	@if "$(word 2, $(MAKECMDGOALS))" == "" (echo ERROR: Shader name not specified! & exit 1)

	$(eval FILE := $(word 2, $(MAKECMDGOALS)))
	$(eval EXT := $(patsubst .%,%,$(suffix $(FILE))))
	$(eval NAME := $(basename $(FILE)))

	@if "$(EXT)" == "" (echo ERROR: Shader file type not specified! & exit 1)

	$(call DELETE_FILE,$(EXT),$(SHD_DIR),$(NAME))

.SECONDEXPANSION:

$(EXE): $(SHADER_OUTPUTS) $(OBJs) $(RESOURCE_OBJ)
	$(CC) $(INCLUDE) $(OBJs) $(RESOURCE_OBJ) -o "$(CURR_DIR)$(EXE)" $(FLAGS_EXE) 

$(OBJ_DIR)%.o: $(SRC_DIR)%.cpp
	$(call CREATE_DIR, $@)
	$(CC) $(INCLUDE) $(FLAGS_OBJ) -c "$<" -o "$@"

ifeq ($(mode), release)
$(OBJs): | $(GENERATED_SHADERS)
endif

$(GPU_DIR)%.spv: $(SHD_DIR)%
	$(call CREATE_DIR, $@)
	$(GLSLC) $(FLAGS_SPV) "$<" -o "$@"

$(GEN_DIR)%.spv.hpp: $(SHD_DIR)%
	$(call CREATE_DIR, $@)
	$(GLSLC) $(FLAGS_SPV) -MF "$@.d" -MT "$@" -mfmt=c "$<" -o "$@.tmp"

	@powershell $(PSFLAGS) -File "$(SCRP_DIR)EmbedShader.ps1" \
		-InputPath "$@.tmp" -OutputPath "$@" -ShaderName "$*"

	@powershell $(PSFLAGS) -Command "Remove-Item '$@.tmp' -Force"

$(RESOURCE_OBJ): $(RES_DIR)$(OUT).rc $(RES_DIR)$(OUT).manifest
	$(call CREATE_DIR, $@)
	@$(WINDRES) "$(RES_DIR)$(OUT).rc" -O coff -o "$(RESOURCE_OBJ)"

CMD_ARGS := $(word 2, $(MAKECMDGOALS)) $(word 3, $(MAKECMDGOALS))
.PHONY: $(CMD_ARGS)
$(CMD_ARGS):
	@:

-include $(OBJ_DEPs) $(SHADER_DEPS)
