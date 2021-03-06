/*
 * (C) Copyright 2013 Emil Ljungdahl
 *
 * This file is part of Openambit.
 *
 * Openambit is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Contributors:
 *
 */
#include <libambit.h>
#include "logentry.h"

LogEntry::LogEntry() :
    personalSettings(NULL),
    logEntry(NULL)
{
}

LogEntry::LogEntry(const LogEntry &other)
{
    device = other.device;
    time = other.time;
    movescountId = other.movescountId;
    deviceInfo = other.deviceInfo;

    if (other.personalSettings != NULL) {
        personalSettings = (ambit_personal_settings_t*)malloc(sizeof(ambit_personal_settings_t));
        memcpy(personalSettings, other.personalSettings, sizeof(ambit_personal_settings_t));

        // need to copy waypoints and routes to avoid re-using the same memory
        if(other.personalSettings->waypoints.data != NULL) {
            personalSettings->waypoints.count = 0;
            libambit_waypoint_append(personalSettings, other.personalSettings->waypoints.data,
                                     other.personalSettings->waypoints.count);
        }
        if(other.personalSettings->routes.data != NULL) {
            ambit_route_t *routes = libambit_route_alloc(other.personalSettings->routes.count);
            personalSettings->routes.data = routes;
            personalSettings->routes.count = other.personalSettings->routes.count;
            for(int r = 0; r < personalSettings->routes.count; r++) {
                ambit_route_t *route = &personalSettings->routes.data[r];
                ambit_route_t *otherRoute = &other.personalSettings->routes.data[r];
                memcpy(route, otherRoute, sizeof(ambit_route_t));

                route->points = (ambit_routepoint_t *) malloc(sizeof(ambit_routepoint_t) * otherRoute->points_count);
                memcpy(route->points, otherRoute->points, sizeof(ambit_routepoint_t) * otherRoute->points_count);
            }
        }
    }
    else {
        personalSettings = NULL;
    }

    if (other.logEntry != NULL) {
        logEntry = (ambit_log_entry_t*)malloc(sizeof(ambit_log_entry_t));
        memcpy(logEntry, other.logEntry, sizeof(ambit_log_entry_t));
        if (other.logEntry->header.activity_name) {
            logEntry->header.activity_name = strdup(other.logEntry->header.activity_name);
        }
        if (other.logEntry->samples != NULL) {
            logEntry->samples = (ambit_log_sample_t*)malloc(sizeof(ambit_log_sample_t)*other.logEntry->samples_count);
            memcpy(logEntry->samples, other.logEntry->samples, sizeof(ambit_log_sample_t)*other.logEntry->samples_count);
            for (unsigned int i=0; i<other.logEntry->samples_count; i++) {
                if (other.logEntry->samples[i].type == ambit_log_sample_type_periodic) {
                    if (other.logEntry->samples[i].u.periodic.values != NULL) {
                        logEntry->samples[i].u.periodic.values = (ambit_log_sample_periodic_value_t*)malloc(sizeof(ambit_log_sample_periodic_value_t)*other.logEntry->samples[i].u.periodic.value_count);
                        memcpy(logEntry->samples[i].u.periodic.values, other.logEntry->samples[i].u.periodic.values, sizeof(ambit_log_sample_periodic_value_t)*other.logEntry->samples[i].u.periodic.value_count);
                    }
                }
                if (other.logEntry->samples[i].type == ambit_log_sample_type_gps_base) {
                    if (other.logEntry->samples[i].u.gps_base.satellites != NULL) {
                        logEntry->samples[i].u.gps_base.satellites = (ambit_log_gps_satellite_t*)malloc(sizeof(ambit_log_gps_satellite_t)*logEntry->samples[i].u.gps_base.satellites_count);
                        memcpy(logEntry->samples[i].u.gps_base.satellites, other.logEntry->samples[i].u.gps_base.satellites, sizeof(ambit_log_gps_satellite_t)*logEntry->samples[i].u.gps_base.satellites_count);
                    }
                }
                if (other.logEntry->samples[i].type == ambit_log_sample_type_unknown) {
                    if (other.logEntry->samples[i].u.unknown.datalen > 0 && other.logEntry->samples[i].u.unknown.data != NULL) {
                        logEntry->samples[i].u.unknown.data = (uint8_t*)malloc(other.logEntry->samples[i].u.unknown.datalen);
                        memcpy(logEntry->samples[i].u.unknown.data, other.logEntry->samples[i].u.unknown.data, other.logEntry->samples[i].u.unknown.datalen);
                    }
                }
            }
        }
    }
}

LogEntry& LogEntry::operator=(const LogEntry &rhs)
{
    LogEntry tmp(rhs);

    std::swap(device, tmp.device);
    std::swap(time, tmp.time);
    std::swap(movescountId, tmp.movescountId);
    std::swap(deviceInfo, tmp.deviceInfo);
    std::swap(personalSettings, tmp.personalSettings);
    std::swap(logEntry, tmp.logEntry);

    return *this;
}

LogEntry::~LogEntry()
{
    if (personalSettings != NULL) {
        libambit_personal_settings_free(personalSettings);
        personalSettings = NULL;
    }

    libambit_log_entry_free(logEntry);

    logEntry = NULL;
}

bool LogEntry::isUploaded()
{
    return this->movescountId != NULL;
}
