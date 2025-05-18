#pragma once

namespace utils
{



template <typename button>
class click_detector 
{
public:
    click_detector() = default;

    void inline update()
    {
        _previous = _current;
        _current = button::isPressed();
        if ( !_previous && _current ) {
            _pressStartTime = millis();
        }
    }

    void inline reset()
    {
        //TODO: 仮置き
        _previous = false;
        _current = false;
        _pressStartTime = 0;
    }

    bool inline isLongPressed()
    {
        if (!_current) {
            return false;
        }

        return millis() - _pressStartTime > LONG_PRESS_MS;
    }

    bool inline isClicked()
    {
        return _previous && !_current;
    }
    
private:
    constexpr static int LONG_PRESS_MS = 5000;
    bool _previous = false;
    bool _current = false;
    uint32_t _pressStartTime = 0;
};


}