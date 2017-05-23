/**
 * @file   InputWrapper.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.12.13
 *
 * @brief  Helper structures for synchronizing input.
 */

#pragma once

namespace viscom {

    struct KeyboardEvent
    {
        /** Holds the key. */
        int key_;
        /** Holds the key's scancode. */
        int scancode_;
        /** Holds the key action. */
        int action_;
        /** Holds modificators. */
        int mods_;

        KeyboardEvent(int key, int scancode, int action, int mods) : key_{key}, scancode_{scancode}, action_{action}, mods_{mods} {}
    };


    struct CharEvent
    {
        /** Holds the character. */
        unsigned int character_;
        /** Holds modificators. */
        int mods_;

        CharEvent(unsigned int character, int mods) : character_{ character }, mods_{ mods } {}
    };


    struct MouseButtonEvent
    {
        /** Holds the button. */
        int button_;
        /** Holds the key action. */
        int action_;

        MouseButtonEvent(int button, int action) : button_{ button }, action_{ action } {}
    };


    struct MousePosEvent
    {
        /** Holds the x coordinate. */
        double x_;
        /** Holds the y coordinate. */
        double y_;

        MousePosEvent(double x, double y) : x_{ x }, y_{ y } {}
    };


    struct MouseScrollEvent
    {
        /** Holds the x offset. */
        double xoffset_;
        /** Holds the y offset. */
        double yoffset_;

        MouseScrollEvent(double xoffset, double yoffset) : xoffset_{ xoffset }, yoffset_{ yoffset } {}
    };
}
