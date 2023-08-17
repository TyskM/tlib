
#pragma once

#include <cmrc/cmrc.hpp>
CMRC_DECLARE(embed);

namespace TLibEmbed
{
    String getFileAsString(const String& path)
    {
        cmrc::file file = cmrc::embed::get_filesystem().open(path);
        String str(file.begin(), file.end());
        return str;
    }
}
