#pragma once

namespace ball {
    /// <summary>
    /// Simple atmosphere model of the Earth
    /// </summary>
    class atmosphere1981 {
    public:
        double density(double height) const;
    };
}