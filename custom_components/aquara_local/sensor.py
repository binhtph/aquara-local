"""Sensors for the Aqara D100: battery, last event, credential count."""

from __future__ import annotations

from datetime import datetime, timezone
from typing import Any

from homeassistant.components.sensor import (
    SensorDeviceClass,
    SensorEntity,
    SensorStateClass,
)
from homeassistant.config_entries import ConfigEntry
from homeassistant.const import PERCENTAGE, EntityCategory
from homeassistant.core import HomeAssistant
from homeassistant.helpers.entity_platform import AddEntitiesCallback

from .const import DOMAIN
from .coordinator import AqaraD100Coordinator, LockInfo
from .entity import AqaraD100Entity


async def async_setup_entry(
    hass: HomeAssistant,
    entry: ConfigEntry,
    async_add_entities: AddEntitiesCallback,
) -> None:
    """Set up D100 sensors."""
    coordinator: AqaraD100Coordinator = hass.data[DOMAIN][entry.entry_id]
    entities: list[SensorEntity] = []
    for lock in coordinator.locks:
        entities.append(AqaraD100BatterySensor(coordinator, lock))
        entities.append(AqaraD100LastEventSensor(coordinator, lock))
        entities.append(AqaraD100CredentialCountSensor(coordinator, lock))
    async_add_entities(entities)


class AqaraD100BatterySensor(AqaraD100Entity, SensorEntity):
    """Reports the lock battery percentage (from the cloud poll)."""

    _attr_device_class = SensorDeviceClass.BATTERY
    _attr_state_class = SensorStateClass.MEASUREMENT
    _attr_native_unit_of_measurement = PERCENTAGE
    _attr_entity_category = EntityCategory.DIAGNOSTIC
    _attr_translation_key = "battery"

    def __init__(self, coordinator: AqaraD100Coordinator, lock: LockInfo) -> None:
        super().__init__(coordinator, lock)
        self._attr_unique_id = f"{lock.did}_battery"

    @property
    def native_value(self) -> int | None:
        state = self.coordinator.data.get(self._lock.did) if self.coordinator.data else None
        return state.battery if state else None


class AqaraD100LastEventSensor(AqaraD100Entity, SensorEntity):
    """Timestamp of the most recent lock event (from the cloud event history).

    This is how Home Assistant learns about opens that happened *outside* HA — e.g.
    a PIN/NFC/manual unlock. The event log is polled fast (~12 s) and decoded into
    who/how; each new event also fires the ``<domain>_event`` bus event for automations.
    """

    _attr_device_class = SensorDeviceClass.TIMESTAMP
    _attr_translation_key = "last_event"

    def __init__(self, coordinator: AqaraD100Coordinator, lock: LockInfo) -> None:
        super().__init__(coordinator, lock)
        self._attr_unique_id = f"{lock.did}_last_event"

    @property
    def native_value(self) -> datetime | None:
        state = self.coordinator.data.get(self._lock.did) if self.coordinator.data else None
        if not state or not state.last_event_ts:
            return None
        return datetime.fromtimestamp(state.last_event_ts / 1000, tz=timezone.utc)

    @property
    def extra_state_attributes(self) -> dict[str, Any] | None:
        state = self.coordinator.data.get(self._lock.did) if self.coordinator.data else None
        if not state:
            return None
        ev = state.last_event or {}
        attrs: dict[str, Any] = {}
        if ev:
            attrs.update(
                {
                    "action": ev.get("action"),
                    "method": ev.get("method"),
                    "user": ev.get("user"),
                    "user_id": ev.get("user_id"),
                }
            )
        if state.last_event_raw:
            attrs["raw"] = state.last_event_raw
        return attrs or None


class AqaraD100CredentialCountSensor(AqaraD100Entity, SensorEntity):
    """How many credentials (PIN/NFC/fingerprint/face) are registered on the lock."""

    _attr_state_class = SensorStateClass.MEASUREMENT
    _attr_entity_category = EntityCategory.DIAGNOSTIC
    _attr_translation_key = "credential_count"

    def __init__(self, coordinator: AqaraD100Coordinator, lock: LockInfo) -> None:
        super().__init__(coordinator, lock)
        self._attr_unique_id = f"{lock.did}_credential_count"

    @property
    def native_value(self) -> int | None:
        state = self.coordinator.data.get(self._lock.did) if self.coordinator.data else None
        return len(state.credentials) if state else None
