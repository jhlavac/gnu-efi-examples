/*
 * Timer application that directly accesses the timer from EFI boot services.
 *
 * Copyright 2018 Red Hat, Inc.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <efi.h>


// fix conversions from a function pointer to (void *)
#undef _cast64_efi_call1
#define _cast64_efi_call1(f, a1) \
        efi_call1((void *) ((UINTN) (f)), (UINT64) (a1))

#undef _cast64_efi_call2
#define _cast64_efi_call2(f, a1, a2) \
        efi_call2((void *) ((UINTN) (f)), (UINT64) (a1), (UINT64) (a2))

#undef _cast64_efi_call3
#define _cast64_efi_call3(f, a1, a2, a3) \
        efi_call3((void *) ((UINTN) (f)), (UINT64) (a1), (UINT64) (a2), \
                                          (UINT64) (a3))

#undef _cast64_efi_call5
#define _cast64_efi_call5(f, a1, a2, a3, a4, a5) \
        efi_call5((void *) ((UINTN) (f)), (UINT64) (a1), (UINT64) (a2), \
                                          (UINT64) (a3), (UINT64) (a4), \
                                          (UINT64) (a5))


#define EFI_TIMER_NS_PER_UNIT 100

#define TIMEOUT_MS            30000
#define TIMEOUT_STEP_MS       100


EFI_STATUS nsleep(const EFI_BOOT_SERVICES *boot_services, UINT64 nano_seconds) {
  EFI_STATUS result;
  EFI_EVENT event;
  result = uefi_call_wrapper(boot_services->CreateEvent, 5,
                             EFI_EVENT_TIMER, EFI_TPL_APPLICATION, NULL, NULL,
                             &event);
  if (result != EFI_SUCCESS) {
    return result;
  }

  // convert ns to 100ns units + round up
  UINT64 units = (nano_seconds + EFI_TIMER_NS_PER_UNIT - 1) /
                 EFI_TIMER_NS_PER_UNIT;

  result = uefi_call_wrapper(boot_services->SetTimer, 3,
                             event, TimerRelative, units);
  if (result != EFI_SUCCESS) {
    return result;
  }

  UINTN index;
  result = uefi_call_wrapper(boot_services->WaitForEvent, 3,
                             1, &event, &index);
  if (result != EFI_SUCCESS) {
    return result;
  }

  result = uefi_call_wrapper(boot_services->CloseEvent, 1,
                             event);
  return result;
}


EFI_STATUS efi_main(EFI_HANDLE image_handle __attribute__((unused)),
                    const EFI_SYSTEM_TABLE *system_table) {
  EFI_STATUS result;
  const EFI_SIMPLE_TEXT_OUT_PROTOCOL *stdout = system_table->ConOut;
  result = uefi_call_wrapper(stdout->Reset, 2,
                             stdout, FALSE);
  if (result != EFI_SUCCESS) {
    return result;
  }

  const EFI_SIMPLE_TEXT_IN_PROTOCOL *stdin = system_table->ConIn;
  result = uefi_call_wrapper(stdin->Reset, 2,
                             stdin, FALSE);
  if (result != EFI_SUCCESS) {
    return result;
  }

  const EFI_BOOT_SERVICES *boot_services = system_table->BootServices;
  CHAR16 text[] = L"\rPress any key to exit (or wait ab.c seconds)...";
  UINT8 counter_position = sizeof("\rPress any key to exit (or wait ") - 1;
  UINT8 a = (TIMEOUT_MS % (1000 * TIMEOUT_STEP_MS)) / (100 * TIMEOUT_STEP_MS);
  UINT8 b = (TIMEOUT_MS % (100 * TIMEOUT_STEP_MS)) / (10 * TIMEOUT_STEP_MS);
  UINT8 c = (TIMEOUT_MS % (10 * TIMEOUT_STEP_MS)) / (1 * TIMEOUT_STEP_MS);
  do {
    text[counter_position] = a? (L'0' + a):(L' ');
    do {
      text[counter_position + sizeof("a") - 1] = L'0' + b;
      do {
        text[counter_position + sizeof("ab.") -1] = L'0' + c;

        result = uefi_call_wrapper(stdout->OutputString, 2,
                                   stdout, text);
        if (result != EFI_SUCCESS) {
          return result;
        }

        EFI_INPUT_KEY key;
        result = uefi_call_wrapper(stdin->ReadKeyStroke, 2,
                                   stdin, &key);
        if (result != EFI_NOT_READY) {
          return result;
        }

        result = nsleep(boot_services, TIMEOUT_STEP_MS * 1000000);
        if (result != EFI_SUCCESS) {
          return result;
        }
      } while (c--);
      c = 9;
    } while (b--);
    b = 9;
  } while (a--);

  return EFI_SUCCESS;
}
