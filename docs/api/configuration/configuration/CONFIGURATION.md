# Configuration

Configuration is a detached control-plane value passed to `Component::configure()`. R0.8 permits configuration from `UNKNOWN` and `STOPPED`; application is atomic and configuration failure does not change lifecycle state.
