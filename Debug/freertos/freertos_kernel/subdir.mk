################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../freertos/freertos_kernel/croutine.c \
../freertos/freertos_kernel/event_groups.c \
../freertos/freertos_kernel/list.c \
../freertos/freertos_kernel/queue.c \
../freertos/freertos_kernel/stream_buffer.c \
../freertos/freertos_kernel/tasks.c \
../freertos/freertos_kernel/timers.c 

C_DEPS += \
./freertos/freertos_kernel/croutine.d \
./freertos/freertos_kernel/event_groups.d \
./freertos/freertos_kernel/list.d \
./freertos/freertos_kernel/queue.d \
./freertos/freertos_kernel/stream_buffer.d \
./freertos/freertos_kernel/tasks.d \
./freertos/freertos_kernel/timers.d 

OBJS += \
./freertos/freertos_kernel/croutine.o \
./freertos/freertos_kernel/event_groups.o \
./freertos/freertos_kernel/list.o \
./freertos/freertos_kernel/queue.o \
./freertos/freertos_kernel/stream_buffer.o \
./freertos/freertos_kernel/tasks.o \
./freertos/freertos_kernel/timers.o 


# Each subdirectory must supply rules for building sources it contributes
freertos/freertos_kernel/%.o: ../freertos/freertos_kernel/%.c freertos/freertos_kernel/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -std=gnu99 -D__REDLIB__ -DCPU_MK64FN1M0VLL12 -DCPU_MK64FN1M0VLL12_cm4 -D_POSIX_SOURCE -DUSE_RTOS=1 -DPRINTF_ADVANCED_ENABLE=1 -DFRDM_K64F -DFREEDOM -DLWIP_DISABLE_PBUF_POOL_SIZE_SANITY_CHECKS=1 -DSERIAL_PORT_TYPE_UART=1 -DSDK_OS_FREE_RTOS -DMCUXPRESSO_SDK -DSDK_DEBUGCONSOLE=1 -DCR_INTEGER_PRINTF -DPRINTF_FLOAT_ENABLE=0 -D__MCUXPRESSO -D__USE_CMSIS -DDEBUG -I"C:\Users\tiori\Documents\MCUXpressoIDE_11.10.0_3148\workspace\Practice2_752964_Mqtt\source" -I"C:\Users\tiori\Documents\MCUXpressoIDE_11.10.0_3148\workspace\Practice2_752964_Mqtt\mdio" -I"C:\Users\tiori\Documents\MCUXpressoIDE_11.10.0_3148\workspace\Practice2_752964_Mqtt\phy" -I"C:\Users\tiori\Documents\MCUXpressoIDE_11.10.0_3148\workspace\Practice2_752964_Mqtt\lwip\src\include\lwip\apps" -I"C:\Users\tiori\Documents\MCUXpressoIDE_11.10.0_3148\workspace\Practice2_752964_Mqtt\lwip\port" -I"C:\Users\tiori\Documents\MCUXpressoIDE_11.10.0_3148\workspace\Practice2_752964_Mqtt\lwip\src" -I"C:\Users\tiori\Documents\MCUXpressoIDE_11.10.0_3148\workspace\Practice2_752964_Mqtt\lwip\src\include" -I"C:\Users\tiori\Documents\MCUXpressoIDE_11.10.0_3148\workspace\Practice2_752964_Mqtt\drivers" -I"C:\Users\tiori\Documents\MCUXpressoIDE_11.10.0_3148\workspace\Practice2_752964_Mqtt\utilities" -I"C:\Users\tiori\Documents\MCUXpressoIDE_11.10.0_3148\workspace\Practice2_752964_Mqtt\device" -I"C:\Users\tiori\Documents\MCUXpressoIDE_11.10.0_3148\workspace\Practice2_752964_Mqtt\component\uart" -I"C:\Users\tiori\Documents\MCUXpressoIDE_11.10.0_3148\workspace\Practice2_752964_Mqtt\component\serial_manager" -I"C:\Users\tiori\Documents\MCUXpressoIDE_11.10.0_3148\workspace\Practice2_752964_Mqtt\component\lists" -I"C:\Users\tiori\Documents\MCUXpressoIDE_11.10.0_3148\workspace\Practice2_752964_Mqtt\CMSIS" -I"C:\Users\tiori\Documents\MCUXpressoIDE_11.10.0_3148\workspace\Practice2_752964_Mqtt\freertos\freertos_kernel\include" -I"C:\Users\tiori\Documents\MCUXpressoIDE_11.10.0_3148\workspace\Practice2_752964_Mqtt\freertos\freertos_kernel\portable\GCC\ARM_CM4F" -I"C:\Users\tiori\Documents\MCUXpressoIDE_11.10.0_3148\workspace\Practice2_752964_Mqtt\board" -O0 -fno-common -g3 -gdwarf-4 -c -ffunction-sections -fdata-sections -ffreestanding -fno-builtin -fmerge-constants -fmacro-prefix-map="$(<D)/"= -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -D__REDLIB__ -fstack-usage -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-freertos-2f-freertos_kernel

clean-freertos-2f-freertos_kernel:
	-$(RM) ./freertos/freertos_kernel/croutine.d ./freertos/freertos_kernel/croutine.o ./freertos/freertos_kernel/event_groups.d ./freertos/freertos_kernel/event_groups.o ./freertos/freertos_kernel/list.d ./freertos/freertos_kernel/list.o ./freertos/freertos_kernel/queue.d ./freertos/freertos_kernel/queue.o ./freertos/freertos_kernel/stream_buffer.d ./freertos/freertos_kernel/stream_buffer.o ./freertos/freertos_kernel/tasks.d ./freertos/freertos_kernel/tasks.o ./freertos/freertos_kernel/timers.d ./freertos/freertos_kernel/timers.o

.PHONY: clean-freertos-2f-freertos_kernel

