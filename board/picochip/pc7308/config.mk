#/*****************************************************************************
# * $picoChipHeaderSubst$
# *****************************************************************************/

#/*!
#* \file config.mk
#* \brief Used during the build process.
#*
#* Copyright (c) 2006-2011 Picochip Ltd
#*
#* This program is free software; you can redistribute it and/or modify
#* it under the terms of the GNU General Public License version 2 as
#* published by the Free Software Foundation.
#*
#* All enquiries to support@picochip.com
#*/

PLATFORM_CPPFLAGS += -DPICOCHIP_PLATFORM_VERSION=\"$(DISTRO_VERSION)\"
