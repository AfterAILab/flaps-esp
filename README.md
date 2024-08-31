# AfterAI Flaps ESP Chip

ESP chip is on Leader PCBs. Follower PCBs do not have ESP chips.

We developed this software based on `ESPMaster` directory of [davidkingsman/split-flap](https://github.com/davidkingsman/split-flap). Here are some of the major changes we have made:

- Adopted React and TypeScript as the development tool of the web UI
- Expanded the range of configuration that can be updated through the web UI
- Added access point (AP) mode, allowing users to connect to the leader directly
- Variable length followers