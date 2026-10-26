#***************************************************************************
# * Project           		: shakti devt board
# * Name of the file	     	: setenv.sh
# * Brief Description of file   : Updates the PATH and sets SHAKTISDK env variables
# * Name of Author    	        : Anand Kumar S
# * Email ID                    : 007334@imail.iitm.ac.in
#
# Copyright (C) 2019  IIT Madras. All rights reserved.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
#
#***************************************************************************/


export PATH=$PATH:$(pwd)/../riscv-tools/tools/riscv32/bin:$(pwd)/../riscv-tools/tools/riscv64/bin
export SHAKTISDK=$(pwd)
