/*
 * Hello World application that directly accesses EFI input and output
 * protocols.
 *
 * Copyright 2018 Red Hat, Inc.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <efi.h>


// fix conversions from a function pointer to (void *)
#undef _cast64_efi_call2
#define _cast64_efi_call2(f, a1, a2) \
        efi_call2((void *) ((UINTN) (f)), (UINT64) (a1), (UINT64) (a2))

#undef _cast64_efi_call3
#define _cast64_efi_call3(f, a1, a2, a3) \
        efi_call3((void *) ((UINTN) (f)), (UINT64) (a1), (UINT64) (a2), \
                                          (UINT64) (a3))


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

  result = uefi_call_wrapper(stdout->OutputString, 2,
                             stdout, L"Hello, world!\r\n"
                                      "\r\n"
                                      "Press any key to exit...");
  if (result != EFI_SUCCESS) {
    return result;
  }

  UINTN index;
  const EFI_BOOT_SERVICES *boot_services = system_table->BootServices;
  result = uefi_call_wrapper(boot_services->WaitForEvent, 3,
                             1, &stdin->WaitForKey, &index);
  if (result != EFI_SUCCESS) {
    return result;
  }

  result = uefi_call_wrapper(stdin->Reset, 2,
                             stdin, FALSE);
  if (result != EFI_SUCCESS) {
    return result;
  }

  return EFI_SUCCESS;
}
