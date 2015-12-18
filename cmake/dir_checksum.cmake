#
# Outputs a checksum of the files in SRC_DIR to STAMP_FILE. cmake targets can
# then depend on this STAMP_FILE to force a rebuild if the directory contents
# changed.
#
# Author: Tim Sjöstrand <tim.sjostrand@gmail.com>
#
cmake_minimum_required(VERSION 3.1)

message(STATUS "SRC_DIR=${SRC_DIR}")
message(STATUS "STAMP_FILE=${STAMP_FILE}")

# Generate file list.
file(GLOB_RECURSE SRC_DIR_FILES "${SRC_DIR}/*")

# Calculate MD5 sum of file list.
string(MD5 SRC_DIR_CHECKSUM "${SRC_DIR_FILES}")

# Make sure old checksum file exists and read it.
file(APPEND ${STAMP_FILE} "")
file(READ ${STAMP_FILE} SRC_DIR_CHECKSUM_PREV)

# Debug
message(STATUS "SRC_DIR_CHECKSUM=${SRC_DIR_CHECKSUM}")
message(STATUS "SRC_DIR_CHECKSUM_PREV=${SRC_DIR_CHECKSUM_PREV}")

# Compare checksums.
string(COMPARE NOTEQUAL "${SRC_DIR_CHECKSUM}" "${SRC_DIR_CHECKSUM_PREV}" STAMP_FILE_NEEDS_UPDATE)
message(STATUS "STAMP_FILE_NEEDS_UPDATE=${STAMP_FILE_NEEDS_UPDATE}")
if(STAMP_FILE_NEEDS_UPDATE)
	message("New contents in ${SRC_DIR}, updating ${STAMP_FILE}...")
	file(WRITE ${STAMP_FILE} ${SRC_DIR_CHECKSUM})
endif()
