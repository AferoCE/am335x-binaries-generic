//
// afwp.h -- Afero hub security code to read and write Wi-Fi passphrase
//
// Copyright (c) 2015-2017 Afero, Inc. All rights reserved.
//

#ifndef __AFWP_H__
#define __AFWP_H__

int af_wp_set_passphrase(uint8_t *passphrase);
int af_wp_get_passphrase(uint8_t *passphrase, int passphraseSize);

#endif // __AFWP_H__
