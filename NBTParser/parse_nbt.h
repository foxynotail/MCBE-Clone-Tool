#define UNUSED(x) (void)(x)


// I could used function overloading, but I want to be sure to get the right data size.
int32_t get_intval(leveldb::Slice slice, uint32_t offset)
{
    int32_t retval = 0;
    // need to switch this to union based like the others.
    for (int i = 0; i < 4; i++) {
        // if I don't do the static cast, the top bit will be sign extended.
        retval |= (static_cast<uint8_t>(slice[offset + i]) << i * 8);
    }
    return retval;
}

long get_longval(leveldb::Slice slice, uint32_t offset)
{
    // initializing a union assigns to the first member.
    // https://en.cppreference.com/w/c/language/struct_initialization
    union {
        long lnum;
        uint8_t bytes[8];
    } retval = { 0 };

    for (int i = 0; i < 8; i++) {
        // if I don't do the static cast, the top bit will be sign extended.
        //retval |= (static_cast<uint8_t>(slice[offset+i])<<i*8);
        retval.bytes[i] = slice[offset + i];
    }
    return retval.lnum;
}

float get_floatval(leveldb::Slice slice, uint32_t offset)
{

    // https://stackoverflow.com/a/36960552/23630
    // I don't understand the issue, but this union enables me to
    // properly extract floats.
    union {
        float fnum;
        uint8_t bytes[4];
    } retval = { 0 };

    for (int i = 0; i < 4; i++) {
        // if I don't do the static cast, the top bit will be sign extended.
        //retval.num |= (static_cast<uint8_t>(slice[offset+i])<<i*8);
        retval.bytes[i] = slice[offset + i];
    }

    return retval.fnum;
}

int16_t get_shortval(leveldb::Slice slice, uint32_t offset)
{
    union {
        int16_t shortnum;
        uint8_t bytes[2];
    } retval = { 0 };

    for (int i = 0; i < 2; i++) {
        retval.bytes[i] = slice[offset + i];
    }
    return retval.shortnum;
}

std::string get_strval(leveldb::Slice slice, uint32_t offset, uint32_t length)
{
    std::string retval;
    retval.reserve(length);

    for (uint32_t i = 0; i < length; i++) {
        retval += slice[offset + i];
    }

    return retval;
}


enum NBT_Tags {
    End = 0,
    Byte = 1,
    Short = 2,
    Int = 3,
    Long = 4,
    Float = 5,
    ByteArray = 7,
    String = 8,
    List = 9,
    Compound = 10,
};
std::map<int, std::string> Tag_Names = {
    {NBT_Tags::End,      "TagEnd"},
    {NBT_Tags::Byte,     "TagByte"},
    {NBT_Tags::Short,    "TagShort"},
    {NBT_Tags::Int,      "TagInt"},
    {NBT_Tags::Long,     "TagLong"},
    {NBT_Tags::Float,    "TagFloat"},
    {NBT_Tags::ByteArray, "TagByteArray"},
    {NBT_Tags::String,   "TagString"},
    {NBT_Tags::List,     "TagList"},
    {NBT_Tags::Compound, "TagCompound"},
};