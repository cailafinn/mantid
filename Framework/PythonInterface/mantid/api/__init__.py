# Mantid Repository : https://github.com/mantidproject/mantid
#
# Copyright &copy; 2018 ISIS Rutherford Appleton Laboratory UKRI,
#     NScD Oak Ridge National Laboratory, European Spallation Source
#     & Institut Laue - Langevin
# SPDX - License - Identifier: GPL - 3.0 +
"""
api
===

Defines Python objects that wrap the C++ API namespace.

"""
from __future__ import absolute_import

###############################################################################
# Load the C++ library
###############################################################################
from mantid.utils import import_mantid_cext

# insert all the classes from _api in the mantid.api namespace
_api = import_mantid_cext('._api', 'mantid.api', globals())

###############################################################################
# Make aliases accessible in this namespace
###############################################################################
from mantid.api._aliases import *

