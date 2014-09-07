################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
I2C_Stellaris_API.obj: ../I2C_Stellaris_API.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccsv5/tools/compiler/arm_5.1.1/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 --abi=eabi -me --include_path="C:/StellarisWare" --include_path="C:/StellarisWare/usblib/ccs-cm4f/Debug" --include_path="C:/StellarisWare/driverlib/ccs-cm4f/Debug" --include_path="C:/ti/ccsv5/tools/compiler/arm_5.1.1/include" -g --gcc --define=PART_LM4F120H5QR --define=ccs="ccs" --define=TARGET_IS_BLIZZARD_RA2 --diag_warning=225 --display_error_number --diag_wrap=off --preproc_with_compile --preproc_dependency="I2C_Stellaris_API.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

PCA9685.obj: ../PCA9685.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccsv5/tools/compiler/arm_5.1.1/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 --abi=eabi -me --include_path="C:/StellarisWare" --include_path="C:/StellarisWare/usblib/ccs-cm4f/Debug" --include_path="C:/StellarisWare/driverlib/ccs-cm4f/Debug" --include_path="C:/ti/ccsv5/tools/compiler/arm_5.1.1/include" -g --gcc --define=PART_LM4F120H5QR --define=ccs="ccs" --define=TARGET_IS_BLIZZARD_RA2 --diag_warning=225 --display_error_number --diag_wrap=off --preproc_with_compile --preproc_dependency="PCA9685.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

lm4f120h5qr_startup_ccs.obj: ../lm4f120h5qr_startup_ccs.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccsv5/tools/compiler/arm_5.1.1/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 --abi=eabi -me --include_path="C:/StellarisWare" --include_path="C:/StellarisWare/usblib/ccs-cm4f/Debug" --include_path="C:/StellarisWare/driverlib/ccs-cm4f/Debug" --include_path="C:/ti/ccsv5/tools/compiler/arm_5.1.1/include" -g --gcc --define=PART_LM4F120H5QR --define=ccs="ccs" --define=TARGET_IS_BLIZZARD_RA2 --diag_warning=225 --display_error_number --diag_wrap=off --preproc_with_compile --preproc_dependency="lm4f120h5qr_startup_ccs.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

main.obj: ../main.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccsv5/tools/compiler/arm_5.1.1/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 --abi=eabi -me --include_path="C:/StellarisWare" --include_path="C:/StellarisWare/usblib/ccs-cm4f/Debug" --include_path="C:/StellarisWare/driverlib/ccs-cm4f/Debug" --include_path="C:/ti/ccsv5/tools/compiler/arm_5.1.1/include" -g --gcc --define=PART_LM4F120H5QR --define=ccs="ccs" --define=TARGET_IS_BLIZZARD_RA2 --diag_warning=225 --display_error_number --diag_wrap=off --preproc_with_compile --preproc_dependency="main.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

str2num.obj: ../str2num.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccsv5/tools/compiler/arm_5.1.1/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 --abi=eabi -me --include_path="C:/StellarisWare" --include_path="C:/StellarisWare/usblib/ccs-cm4f/Debug" --include_path="C:/StellarisWare/driverlib/ccs-cm4f/Debug" --include_path="C:/ti/ccsv5/tools/compiler/arm_5.1.1/include" -g --gcc --define=PART_LM4F120H5QR --define=ccs="ccs" --define=TARGET_IS_BLIZZARD_RA2 --diag_warning=225 --display_error_number --diag_wrap=off --preproc_with_compile --preproc_dependency="str2num.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

uartstdio.obj: ../uartstdio.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccsv5/tools/compiler/arm_5.1.1/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 --abi=eabi -me --include_path="C:/StellarisWare" --include_path="C:/StellarisWare/usblib/ccs-cm4f/Debug" --include_path="C:/StellarisWare/driverlib/ccs-cm4f/Debug" --include_path="C:/ti/ccsv5/tools/compiler/arm_5.1.1/include" -g --gcc --define=PART_LM4F120H5QR --define=ccs="ccs" --define=TARGET_IS_BLIZZARD_RA2 --diag_warning=225 --display_error_number --diag_wrap=off --preproc_with_compile --preproc_dependency="uartstdio.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

usb_handlers.obj: ../usb_handlers.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccsv5/tools/compiler/arm_5.1.1/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 --abi=eabi -me --include_path="C:/StellarisWare" --include_path="C:/StellarisWare/usblib/ccs-cm4f/Debug" --include_path="C:/StellarisWare/driverlib/ccs-cm4f/Debug" --include_path="C:/ti/ccsv5/tools/compiler/arm_5.1.1/include" -g --gcc --define=PART_LM4F120H5QR --define=ccs="ccs" --define=TARGET_IS_BLIZZARD_RA2 --diag_warning=225 --display_error_number --diag_wrap=off --preproc_with_compile --preproc_dependency="usb_handlers.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

usb_structs.obj: ../usb_structs.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccsv5/tools/compiler/arm_5.1.1/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 --abi=eabi -me --include_path="C:/StellarisWare" --include_path="C:/StellarisWare/usblib/ccs-cm4f/Debug" --include_path="C:/StellarisWare/driverlib/ccs-cm4f/Debug" --include_path="C:/ti/ccsv5/tools/compiler/arm_5.1.1/include" -g --gcc --define=PART_LM4F120H5QR --define=ccs="ccs" --define=TARGET_IS_BLIZZARD_RA2 --diag_warning=225 --display_error_number --diag_wrap=off --preproc_with_compile --preproc_dependency="usb_structs.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

waveform.obj: ../waveform.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccsv5/tools/compiler/arm_5.1.1/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 --abi=eabi -me --include_path="C:/StellarisWare" --include_path="C:/StellarisWare/usblib/ccs-cm4f/Debug" --include_path="C:/StellarisWare/driverlib/ccs-cm4f/Debug" --include_path="C:/ti/ccsv5/tools/compiler/arm_5.1.1/include" -g --gcc --define=PART_LM4F120H5QR --define=ccs="ccs" --define=TARGET_IS_BLIZZARD_RA2 --diag_warning=225 --display_error_number --diag_wrap=off --preproc_with_compile --preproc_dependency="waveform.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


