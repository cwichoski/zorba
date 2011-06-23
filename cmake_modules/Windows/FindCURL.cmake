# Copyright 2010 The FLWOR Foundation.
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
# http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# - Find the native cURL headers and libraries on Windows
#
# This is a proxy module that calls the FindCURL.cmake module shipped with
# CMake. But before we try to guess where cURL might be on the user's machine.
# The user should provide ZORBA_THIRD_PARTY_REQUIREMENTS which is a path where
# the cURL directory can be found. The cURL directory must have "curl"
# (case insensitive) in its name.
#
# This module helps the Windows user to avoid providing the following two
# variables when building Zorba:
# -D CURL_INCLUDE_DIR="path_to_3rd_party_dir\*curl*\include"
# -D CURL_LIBRARY="path_to_3rd_party_dir\*curl*\libcurl_imp.lib"
#
# See the FindCURL.cmake module shipped with CMake for more information.

FIND_PACKAGE_WIN32(CURL CURL_FOUND curl)

IF (CURL_FOUND)

  # find the needed DLL's
  FIND_PACKAGE_DLLS_WIN32 (${FOUND_LOCATION} "curllib.dll;libeay32.dll;openldap.dll;ssleay32.dll")

  # find additional DLL's
  FIND_DLL (libsasl.dll)

ENDIF (CURL_FOUND)
