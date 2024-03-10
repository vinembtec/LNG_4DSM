################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
Peripheral\ Modules/SX1276_RADIO/radio.obj: ../Peripheral\ Modules/SX1276_RADIO/radio.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: MSP432 Compiler'
	"C:/ti/ccs1010/ccs/tools/compiler/ti-cgt-arm_20.2.1.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me -Ooff --opt_for_speed=1 --include_path="C:/ti/ccs1010/ccs/ccs_base/arm/include" --include_path="C:/ti/ccs1010/ccs/ccs_base/arm/include/CMSIS" --include_path="C:/ti/ccs1010/ccs/tools/compiler/ti-cgt-arm_20.2.1.LTS/include" --advice:power="all" -g --gcc --define=__MSP432P401R__ --define=TARGET_IS_MSP432P4XX --define=ccs --display_error_number --diag_wrap=off --diag_warning=225 --abi=eabi --preproc_with_compile --preproc_dependency="Peripheral Modules/SX1276_RADIO/radio.d_raw" --obj_directory="Peripheral Modules/SX1276_RADIO" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

Peripheral\ Modules/SX1276_RADIO/radio_test.obj: ../Peripheral\ Modules/SX1276_RADIO/radio_test.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: MSP432 Compiler'
	"C:/ti/ccs1010/ccs/tools/compiler/ti-cgt-arm_20.2.1.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me -Ooff --opt_for_speed=1 --include_path="C:/ti/ccs1010/ccs/ccs_base/arm/include" --include_path="C:/ti/ccs1010/ccs/ccs_base/arm/include/CMSIS" --include_path="C:/ti/ccs1010/ccs/tools/compiler/ti-cgt-arm_20.2.1.LTS/include" --advice:power="all" -g --gcc --define=__MSP432P401R__ --define=TARGET_IS_MSP432P4XX --define=ccs --display_error_number --diag_wrap=off --diag_warning=225 --abi=eabi --preproc_with_compile --preproc_dependency="Peripheral Modules/SX1276_RADIO/radio_test.d_raw" --obj_directory="Peripheral Modules/SX1276_RADIO" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

Peripheral\ Modules/SX1276_RADIO/sx1276-Hal.obj: ../Peripheral\ Modules/SX1276_RADIO/sx1276-Hal.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: MSP432 Compiler'
	"C:/ti/ccs1010/ccs/tools/compiler/ti-cgt-arm_20.2.1.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me -Ooff --opt_for_speed=1 --include_path="C:/ti/ccs1010/ccs/ccs_base/arm/include" --include_path="C:/ti/ccs1010/ccs/ccs_base/arm/include/CMSIS" --include_path="C:/ti/ccs1010/ccs/tools/compiler/ti-cgt-arm_20.2.1.LTS/include" --advice:power="all" -g --gcc --define=__MSP432P401R__ --define=TARGET_IS_MSP432P4XX --define=ccs --display_error_number --diag_wrap=off --diag_warning=225 --abi=eabi --preproc_with_compile --preproc_dependency="Peripheral Modules/SX1276_RADIO/sx1276-Hal.d_raw" --obj_directory="Peripheral Modules/SX1276_RADIO" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

Peripheral\ Modules/SX1276_RADIO/sx1276-LoRa.obj: ../Peripheral\ Modules/SX1276_RADIO/sx1276-LoRa.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: MSP432 Compiler'
	"C:/ti/ccs1010/ccs/tools/compiler/ti-cgt-arm_20.2.1.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me -Ooff --opt_for_speed=1 --include_path="C:/ti/ccs1010/ccs/ccs_base/arm/include" --include_path="C:/ti/ccs1010/ccs/ccs_base/arm/include/CMSIS" --include_path="C:/ti/ccs1010/ccs/tools/compiler/ti-cgt-arm_20.2.1.LTS/include" --advice:power="all" -g --gcc --define=__MSP432P401R__ --define=TARGET_IS_MSP432P4XX --define=ccs --display_error_number --diag_wrap=off --diag_warning=225 --abi=eabi --preproc_with_compile --preproc_dependency="Peripheral Modules/SX1276_RADIO/sx1276-LoRa.d_raw" --obj_directory="Peripheral Modules/SX1276_RADIO" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

Peripheral\ Modules/SX1276_RADIO/sx1276-LoRaMisc.obj: ../Peripheral\ Modules/SX1276_RADIO/sx1276-LoRaMisc.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: MSP432 Compiler'
	"C:/ti/ccs1010/ccs/tools/compiler/ti-cgt-arm_20.2.1.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me -Ooff --opt_for_speed=1 --include_path="C:/ti/ccs1010/ccs/ccs_base/arm/include" --include_path="C:/ti/ccs1010/ccs/ccs_base/arm/include/CMSIS" --include_path="C:/ti/ccs1010/ccs/tools/compiler/ti-cgt-arm_20.2.1.LTS/include" --advice:power="all" -g --gcc --define=__MSP432P401R__ --define=TARGET_IS_MSP432P4XX --define=ccs --display_error_number --diag_wrap=off --diag_warning=225 --abi=eabi --preproc_with_compile --preproc_dependency="Peripheral Modules/SX1276_RADIO/sx1276-LoRaMisc.d_raw" --obj_directory="Peripheral Modules/SX1276_RADIO" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

Peripheral\ Modules/SX1276_RADIO/sx1276.obj: ../Peripheral\ Modules/SX1276_RADIO/sx1276.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: MSP432 Compiler'
	"C:/ti/ccs1010/ccs/tools/compiler/ti-cgt-arm_20.2.1.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me -Ooff --opt_for_speed=1 --include_path="C:/ti/ccs1010/ccs/ccs_base/arm/include" --include_path="C:/ti/ccs1010/ccs/ccs_base/arm/include/CMSIS" --include_path="C:/ti/ccs1010/ccs/tools/compiler/ti-cgt-arm_20.2.1.LTS/include" --advice:power="all" -g --gcc --define=__MSP432P401R__ --define=TARGET_IS_MSP432P4XX --define=ccs --display_error_number --diag_wrap=off --diag_warning=225 --abi=eabi --preproc_with_compile --preproc_dependency="Peripheral Modules/SX1276_RADIO/sx1276.d_raw" --obj_directory="Peripheral Modules/SX1276_RADIO" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


