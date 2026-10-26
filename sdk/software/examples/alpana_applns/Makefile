 #Created by Sathya Narayanan N
# Email id: sathya281@gmail.com
	
#   Copyright (C) 2019  IIT Madras. All rights reserved.

#This program is free software: you can redistribute it and/or modify
#it under the terms of the GNU General Public License as published by
#the Free Software Foundation, either version 3 of the License, or
#(at your option) any later version.

#This program is distributed in the hope that it will be useful,
#but WITHOUT ANY WARRANTY; without even the implied warranty of
#MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>

CLEAR ?=
directories := $(shell find $(PWD)/* -maxdepth 0 -type d -printf "%f\n")

#if there are no *.o files to delete, you might still end up with an error message. To suppress these, add the following

all:
	@$(foreach dir,$(directories),(cd $(dir) && make all PROGRAM=$(dir))&&):


clean:
ifeq ($(PROGRAM),)
	@find -type d -name output -a -prune -exec rm -rf {} \;
	@echo "cleaned"
else
	@rm -fr ./$(PROGRAM)/output
	@echo "cleaned"
endif

#    2>/dev/null pipes any error message to /dev/null - so you won't see any errors
#    the - in front of the command makes sure that make ignores a non-zero return code


