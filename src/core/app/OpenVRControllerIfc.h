/**
 * @file   OpenVRControllerIfc.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2019.02.21
 *
 * @brief  Controller interface for all OpenVR related input.
 */

#pragma once

#include "OpenVR.h"

namespace viscom::ovr {

    class OpenVRControllerIfc
    {
    public:
        /** Initializes OpenVR for controller usage. */
        virtual bool InitialiseVR() = 0;
        /** Initializes the display edges either by file or with default values if no displayEdges txt is found. */
        virtual bool InitialiseDisplayVR() = 0;
        /** Calibrates the display edges by selected method. */
        virtual bool CalibrateVR(CalibrateMethod method) = 0;
        /** Returns a DeviceInfo vector with the connected devices. */
        virtual const std::vector<DeviceInfo>& GetConnectedDevices() const = 0;
        /** Returns the position to a given tracked device id. */
        virtual const glm::vec3& GetControllerPosition(std::uint32_t trackedDeviceId) const = 0;
        /** Returns the z-vector to a given tracked device id. */
        virtual const glm::vec3& GetControllerDirection(std::uint32_t trackedDeviceId) const = 0;
        /** Returns the rotation to a given tracked device id. */
        virtual const glm::quat& GetControllerOrientation(std::uint32_t trackedDeviceId) const = 0;
        /** Returns the display pointing position for a given tracked device id. */
        virtual const glm::vec2& GetDisplayPointerPosition(std::uint32_t trackedDeviceId) const = 0;

        virtual bool ControllerButtonPressedCallback(std::uint32_t trackedDeviceId, std::size_t buttonid) = 0;
        virtual bool ControllerButtonTouchedCallback(std::uint32_t trackedDeviceId, std::size_t buttonid) = 0;
        virtual bool ControllerButtonPressReleasedCallback(std::uint32_t trackedDeviceId, std::size_t buttonid) = 0;
        virtual bool ControllerButtonTouchReleasedCallback(std::uint32_t trackedDeviceId, std::size_t buttonid) = 0;

        /** Fills buttonstate and axisvalues for a given tracked device and buttonid. */
        virtual void GetControllerButtonState(std::uint32_t trackedDeviceId, std::size_t buttonid, glm::vec2& axisvalues, ButtonState& buttonstate) const = 0;

        virtual std::vector<std::string> OutputDevices() const = 0;

    protected:
        static const std::vector<DeviceInfo> noDeviceInfo_;
        static const glm::vec3 zeroVec3_;
        static const glm::vec2 zeroVec2_;
        static const glm::quat identityQuat_;
    };

    class OpenVRControllerDummy : public OpenVRControllerIfc
    {
        virtual bool InitialiseVR() override { return false; }
        virtual bool InitialiseDisplayVR() override { return false; }
        virtual bool CalibrateVR(CalibrateMethod method) override { return false; }
        virtual const std::vector<DeviceInfo>& GetConnectedDevices() const override { return noDeviceInfo_; }
        virtual const glm::vec3& GetControllerPosition(std::uint32_t trackedDeviceId) const override { return zeroVec3_; }
        virtual const glm::vec3& GetControllerDirection(std::uint32_t trackedDeviceId) const override { return zeroVec3_; }
        virtual const glm::quat& GetControllerOrientation(std::uint32_t trackedDeviceId) const override { return identityQuat_; }
        virtual const glm::vec2& GetDisplayPointerPosition(std::uint32_t trackedDeviceId) const override { return zeroVec2_; }

        virtual bool ControllerButtonPressedCallback(std::uint32_t trackedDeviceId, std::size_t buttonid) override { return false; }
        virtual bool ControllerButtonTouchedCallback(std::uint32_t trackedDeviceId, std::size_t buttonid) override { return false; }
        virtual bool ControllerButtonPressReleasedCallback(std::uint32_t trackedDeviceId, std::size_t buttonid) override { return false; }
        virtual bool ControllerButtonTouchReleasedCallback(std::uint32_t trackedDeviceId, std::size_t buttonid)  override { return false; }

        virtual void GetControllerButtonState(std::uint32_t trackedDeviceId, std::size_t buttonid, glm::vec2& axisvalues, ButtonState& buttonstate) const override {};

        virtual std::vector<std::string> OutputDevices() const override { return std::vector<std::string>(); }
    };
}
