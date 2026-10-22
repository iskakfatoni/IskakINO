/*
 * src/storage/IskakINO_StorageModule.h
 * Adapter tipis supaya IskakStorage (instance global) bisa didaftarkan ke
 * IskakINO_Kernel. Storage bersifat synchronous (tidak ada state machine
 * yang perlu di-"tick" tiap loop()), jadi update() sengaja kosong -- adapter
 * ini murni supaya IskakStorage.begin(namespace, debug) ikut terpanggil
 * otomatis lewat IskakINO.begin().
 */

#ifndef ISKAKINO_STORAGE_MODULE_H
#define ISKAKINO_STORAGE_MODULE_H

#include "../core/IskakINO_Module.h"
#include "IskakINO_Storage.h"

class IskakINO_StorageModule : public IskakINO_Module {
  private:
    const char* _ns;
    bool _debug;

  public:
    explicit IskakINO_StorageModule(const char* ns = "iskak_store", bool debug = false)
        : _ns(ns), _debug(debug) {}

    void begin() override { IskakStorage.begin(_ns, _debug); }
    // update() sengaja tidak di-override (default no-op dari IskakINO_Module)
    const char* moduleName() const override { return "Storage"; }
};

#endif
