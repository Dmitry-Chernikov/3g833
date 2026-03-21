//
// Created by dmitry on 16.02.2026.
//

/** @file EEPROMManager.h
 * @brief Create by dmitry on 16.02.2026
 * @author : Dmitry Chernikov
 * @date : 16.02.2026
 * @project : 3g833
 * Header Class EEPROMManager
*/
#pragma once


class EEPROMManager {
public:
    // Константы для хранения данных в памяти
    static constexpr int STORAGE_ADDR = 0;

    bool load(SystemState &state);

    void save(const SystemState &state);

    bool needsSave(const SystemState &current, const SystemState &buffer);

    void clear();

private:
    bool validateChecksum(const SystemState &data);

    uint16_t calculateChecksum(const SystemState &data);
};