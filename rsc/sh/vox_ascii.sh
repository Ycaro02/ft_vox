#!/bin/bash

source rsc/sh/color.sh

ascii_vox() {
	# font name : ANSI Shadow
	echo -e "\n"
	echo -e "${LIGHT_RED}                                                        ██╗   ██╗   ██████╗   ██╗  ██╗${RESET}"
	echo -e "${LIGHT_RED}                                                        ██║   ██║  ██╔═══██╗  ╚██╗██╔╝${RESET}"
	echo -e "${LIGHT_RED}                                                        ██║   ██║  ██║   ██║   ╚███╔╝ ${RESET}"
	echo -e "${LIGHT_RED}                                                        ╚██╗ ██╔╝  ██║   ██║   ██╔██╗ ${RESET}"
	echo -e "${LIGHT_RED}                                                         ╚████╔╝   ╚██████╔╝  ██╔╝ ██╗${RESET}"
	echo -e "${LIGHT_RED}                                                          ╚═══╝     ╚═════╝   ╚═╝  ╚═╝${RESET}\n\n"

}

minecraft_world() {
# font name : big
echo -e "${LIGHT_GREEN}   __  __  __ _                            __ _                        _     _                                   _              __  ${RESET}"
echo -e "${LIGHT_GREEN}  / / |  \\/  (_)                          / _| |                      | |   | |                                 | |             \\ \\ ${RESET}"
echo -e "${LIGHT_GREEN} | |  | \\  / |_|_ __   ___  ___ _ __ __ _| |_| |_  __      _____  _ __| | __| |   __ _  ___ _ __   ___ _ __ __ _| |_ ___  _ __   | |${RESET}"
echo -e "${LIGHT_GREEN} | |  | |\\/| | | '_ \\ / _ \\/ __| '__/ _\` |  _| __| \\ \\ /\\ / / _ \\| '__| |/ _\` |  / _\` |/ _ \\ '_ \\ / _ \\ '__/ _\` | __/ _ \\| '__|  | |${RESET}"
echo -e "${LIGHT_GREEN} | |  | |  | | | | | |  __/ (__| | | (_| | | | |_   \\ V  V / (_) | |  | | (_| | | (_| |  __/ | | |  __/ | | (_| | || (_) | |     | |${RESET}"
echo -e "${LIGHT_GREEN} | |  |_|  |_|_|_| |_|\\___|\\___|_|  \\__,_|_|  \\__|   \\_/\\_/ \\___/|_|  |_|\\__,_|  \\__, |\\___|_| |_|\\___|_|  \\__,_|\\__\\___/|_|     | |${RESET}"
echo -e "${LIGHT_GREEN}  \\_\\                                                                             __/ |                                         /_/ ${RESET}"
echo -e "${LIGHT_GREEN}                                                                                 |___/                                              ${RESET}\n\n"
}


ascii_vox

minecraft_world
