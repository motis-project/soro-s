# This simply appends
#  export { Module, FS, IDBFS }
# to the file read from SOURCE and writes it to DEST

FILE(READ ${SOURCE} SOURCE_CONTENTS)
FILE(WRITE ${DEST} "${SOURCE_CONTENTS} \nexport { Module, FS, IDBFS }\n")
