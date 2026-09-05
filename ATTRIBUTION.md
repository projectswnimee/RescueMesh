# Source and contributions

RescueMesh is collaborative engineering work. This repository is maintained by
Nimesha Nanayakkara (`projectswnimee`) as an authorized project copy.

## Original source

- [Team repository](https://github.com/HiranGeeth/RescueMesh-Disaster-Rescue_Long-Range-Communication-System)
- Imported source revision: `a151df3` (see the upstream history for full authorship)
- Original [MIT licence](LICENSE): copyright 2025 Hiran Geeth, preserved unchanged

The upstream commit history credits Hiran Geeth / Hiran Geeth Dharmapala and
Lasindu Viduranga. The hardware documentation explicitly credits Lasindu
Viduranga for the power-path circuit and enclosure work. Those credits remain
in their original files.

## This publication

This repository starts with a reviewed source snapshot instead of copying the
upstream history, which contains hard-coded credentials. The publication work
covers README organization, source attribution, local-configuration examples,
and competition recognition. A new import commit does not imply sole authorship
of the firmware, PCB, enclosure or simulations.

Individual engineering roles beyond those documented above will be recorded
when confirmed by the team. No roles have been assigned based on who uploaded
the repository.

## Recognition

Semi-finalist in the SL IoT competition, Sri Lanka, as reported by project
collaborator Nimesha Nanayakkara. The edition/year and certificate or official
announcement are not yet included in this repository.

## Publication changes

- Replaced Wi-Fi credentials and a webhook endpoint with an ignored local
  `secrets.h` and a committed `secrets.example.h` template.
- Replaced the field-node key whitelist with clearly labelled demonstration keys.
- Moved the terrain-download API key to `OPENTOPOGRAPHY_API_KEY` in the local environment.
- Retained the original source folders and hardware design assets.
- Excluded local KiCad preference/cache files through `.gitignore`.
- Corrected the project description to distinguish custom LoRa messaging from
  a verified LoRaWAN implementation, and separated design claims from measurements.
