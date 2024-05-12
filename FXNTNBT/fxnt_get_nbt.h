int fxnt_getNBTInit(leveldb::Slice slice, int offset, int parent, fxNBT& data);

int fxnt_getNBTData(leveldb::Slice slice, int id, int initOffset, int offset, int parent, std::string tagname, fxNBT& data)
{
    fxNBT prevData = data.map[data.key];
    data.key++;
    int thisKey = data.key;
    fxNBT thisData = data.map[data.key];
    thisData.init_offset = initOffset;
    thisData.key_offset = offset;
    thisData.key = data.key;
    thisData.tagname = tagname;
    thisData.parent = parent;
    thisData.group = data.map[thisData.parent].tagname;
    thisData.type = "value";

    switch (id) {
    case NBT_Tags::Compound: {

        thisData.type = "Compound";

        if (prevData.tagname != thisData.tagname) thisData.group = tagname;

        if (prevData.parent > 0) thisData.data_type = "item";
        else thisData.data_type = "tile";

        int entries = 0;
        data.map[thisKey] = thisData;
        while (slice[offset] != NBT_Tags::End) {
            entries++;            
            offset = fxnt_getNBTInit(slice, offset, thisKey, data);
        }
        thisData.entries = entries;

        break;
    }

    case NBT_Tags::List: {

        //std::cout << "Type: List" << std::endl;
        thisData.type = "List";
        thisData.group = tagname;
        thisData.key_offset = offset;

        int8_t tagid = (int8_t)slice[offset];
        thisData.tagID = tagid;
        offset += 1;

        thisData.val_offset = offset;
        int32_t length = get_intval(slice, offset);
        thisData.entries = length;
        offset += 4;
        thisData.val_length = offset - thisData.val_offset;

        // Get inital offset of first item in the list
        data.map[data.key] = thisData;
        for (int i = 0; i < length; i++) {
            offset = fxnt_getNBTData(slice, tagid, offset, offset, thisData.key, tagname, data);
        }

        break;
    }
    case NBT_Tags::String: {

        //std::cout << "Type: String" << std::endl;
        thisData.data_type = "String";
        int16_t length = get_shortval(slice, offset);
        thisData.length = length;
        thisData.key_offset = offset;
        offset += 2;        
        thisData.val_offset = offset;
        thisData.str_val = get_strval(slice, offset, length);
        offset += length;
        thisData.val_length = offset - thisData.val_offset;
        break;
    }

    case NBT_Tags::Byte: {

        //std::cout << "Type: Byte" << std::endl;
        thisData.data_type = "Byte";
        thisData.length = 1;
        thisData.val_offset = offset;
        thisData.byte_val = (int8_t)slice[offset];
        offset += 1;
        thisData.val_length = offset - thisData.val_offset;
        break;
    }

    case NBT_Tags::Short: {

        //std::cout << "Type: Short" << std::endl;
        thisData.data_type = "Short";
        thisData.length = 2;
        thisData.val_offset = offset;
        thisData.short_val = get_shortval(slice, offset);
        offset += 2;
        thisData.val_length = offset - thisData.val_offset;
        break;
    }

    case NBT_Tags::Int: {

        //std::cout << "Type: Int" << std::endl;
        thisData.data_type = "Int";
        thisData.length = 4;
        thisData.val_offset = offset;
        thisData.int_val = get_intval(slice, offset);
        offset += 4;
        thisData.val_length = offset - thisData.val_offset;
        break;
    }

    case NBT_Tags::Long: {

        //std::cout << "Type: Long" << std::endl;
        thisData.data_type = "Long";
        thisData.length = 8;
        thisData.val_offset = offset;
        thisData.long_val = get_longval(slice, offset);
        offset += 8;
        thisData.val_length = offset - thisData.val_offset;
        break;
    }
    case NBT_Tags::Float: {

        //std::cout << "Type: Float" << std::endl;
        thisData.data_type = "Float";
        thisData.length = 4;
        thisData.val_offset = offset;
        thisData.float_val = get_floatval(slice, offset);
        offset += 4;
        thisData.val_length = offset - thisData.val_offset;
        break;
    }

    case NBT_Tags::ByteArray: {
        //std::cout << "Type: ByteArray" << std::endl;
        thisData.data_type = "ByteArray";
        int32_t length = get_intval(slice, offset);
        thisData.length = length;
        thisData.key_offset = offset;
        offset += 4;
        thisData.val_offset = offset;
        thisData.str_val = slice.ToString().substr(offset, length);
        offset += length;
        thisData.val_length = offset - thisData.val_offset;
        break;
    }
    default:
        //std::cout << "Type: Other" << std::endl;
        thisData.type = "Other";
        break;
    }

    thisData.offset = offset;
    int initLength = offset - thisData.init_offset;
    thisData.init_length = initLength;
    int keyLength = offset - thisData.key_offset;
    thisData.key_length = keyLength;
    data.map[thisKey] = thisData;
    if (id == NBT_Tags::Compound) {
        // this is to skip over the NBT_Tags::End.
        offset += 1;
    }
    if (id == NBT_Tags::End) {
        //std::cout << "END" << std::endl;
    }
    return offset;

}

int fxnt_getNBTInit(leveldb::Slice slice, int offset, int parent, fxNBT& data)
{
    //std::cout << "SOffset: " << offset << std::endl;
    //std::cout << "Type: Init" << std::endl;
    // first byte is the nbt "node number". tells you what the next tag is.
    int initOffset = offset;
    int id = slice[offset];
    offset += 1;

    // all nbt tags can have a name.
    int offsetSpread = offset + 1;
    int length = (slice[offset]) | (slice[offsetSpread] << 8);
    offset += 2;

    std::string tagname = get_strval(slice, offset, length);
    offset += length;

    offset = fxnt_getNBTData(slice, id, initOffset, offset, parent, tagname, data);

    return offset;
}


int fxnt_getNBT(leveldb::Slice slice, int offset, fxNBT& data) {
    offset = fxnt_getNBTInit(slice, offset, 0, data);
    return offset;
}