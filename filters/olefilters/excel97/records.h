/* This file is part of the KDE project

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef RECORDS_H
#define RECORDS_H

struct record {
  Q_UINT16 opcode;
  bool (XMLTree::*func)(Q_UINT16 size, QDataStream& body);
};

const record biff[] = {
  { 0x0022, &XMLTree::_1904 },
  { 0x0087, &XMLTree::_addin },
  { 0x00c2, &XMLTree::_addmenu },
  { 0x0221, &XMLTree::_array },
  { 0x009e, &XMLTree::_autofilter },
  { 0x009d, &XMLTree::_autofilterinfo },
  { 0x0040, &XMLTree::_backup },
  { 0x0201, &XMLTree::_blank },
  { 0x0809, &XMLTree::_bof },
  { 0x00da, &XMLTree::_bookbool },
  { 0x0205, &XMLTree::_boolerr },
  { 0x0029, &XMLTree::_bottommargin },
  { 0x0085, &XMLTree::_boundsheet },
  { 0x000c, &XMLTree::_calccount },
  { 0x000d, &XMLTree::_calcmode },
  { 0x01b1, &XMLTree::_cf },
  { 0x01b0, &XMLTree::_condfmt },
  { 0x0042, &XMLTree::_codename }, 
  { 0X0042, &XMLTree::_codepage }, 
  { 0x007d, &XMLTree::_colinfo },
  { 0x003c, &XMLTree::_cont },
  { 0x00a9, &XMLTree::_coordlist },
  { 0x008c, &XMLTree::_country },
  { 0x005a, &XMLTree::_crn },
  { 0x00d7, &XMLTree::_dbcell },
  { 0x0050, &XMLTree::_dcon },
  { 0x01b5, &XMLTree::_dconbin },
  { 0x0052, &XMLTree::_dconname },
  { 0x0051, &XMLTree::_dconref },
  { 0x0225, &XMLTree::_defaultrowheight },
  { 0x0055, &XMLTree::_defcolwidth },
  { 0x00c3, &XMLTree::_delmenu },
  { 0x0010, &XMLTree::_delta },
  { 0x0200, &XMLTree::_dimensions },
  { 0x00b8, &XMLTree::_docroute },
  { 0x0161, &XMLTree::_dsf },
  { 0x01be, &XMLTree::_dv },
  { 0x01b2, &XMLTree::_dval },
  { 0x0088, &XMLTree::_edg },
  { 0x000a, &XMLTree::_eof },
  { 0x0016, &XMLTree::_externcount },
  { 0x0223, &XMLTree::_externname },
  { 0x0017, &XMLTree::_externsheet },
  { 0x00ff, &XMLTree::_extsst },
  { 0x002f, &XMLTree::_filepass },
  { 0x005b, &XMLTree::_filesharing },
  { 0x01a5, &XMLTree::_filesharing2 },
  { 0x009b, &XMLTree::_filtermode },
  { 0x009c, &XMLTree::_fngroupcount },
  { 0x009a, &XMLTree::_fngroupname },
  { 0x0031, &XMLTree::_font },
  { 0x0015, &XMLTree::_footer },
  { 0x041e, &XMLTree::_format },
  { 0x0406, &XMLTree::_formula },
  { 0x00ab, &XMLTree::_gcw },
  { 0x0082, &XMLTree::_gridset },
  { 0x0080, &XMLTree::_guts },
  { 0x0083, &XMLTree::_hcenter },
  { 0x0014, &XMLTree::_header },
  { 0x008d, &XMLTree::_hideobj },
  { 0x01b8, &XMLTree::_hlink },
  { 0x001b, &XMLTree::_horizontalpagebreaks },
  { 0x007f, &XMLTree::_imdata },
  { 0x020b, &XMLTree::_index },
  { 0x00e2, &XMLTree::_interfaceend },
  { 0x00e1, &XMLTree::_interfacehdr },
  { 0x0011, &XMLTree::_iteration },
  { 0x0204, &XMLTree::_label },
  { 0x00fd, &XMLTree::_labelsst },
  { 0x0026, &XMLTree::_leftmargin },
  { 0x0095, &XMLTree::_lhngraph },
  { 0x0094, &XMLTree::_lhrecord },
  { 0x0098, &XMLTree::_lpr },
  { 0x00c1, &XMLTree::_mms },
  { 0x00ec, &XMLTree::_msodrawing },
  { 0x00eb, &XMLTree::_msodrawinggroup },
  { 0x00ed, &XMLTree::_msodrawingselection },
  { 0x00be, &XMLTree::_mulblank },
  { 0x00bd, &XMLTree::_mulrk },
  { 0x0218, &XMLTree::_name },
  { 0x001c, &XMLTree::_note },
  { 0x0203, &XMLTree::_number },
  { 0x005d, &XMLTree::_obj },
  { 0x0063, &XMLTree::_objprotect },
  { 0x00d3, &XMLTree::_obproj },
  { 0x00de, &XMLTree::_olesize },
  { 0x0092, &XMLTree::_palette },
  { 0x0041, &XMLTree::_pane },
  { 0x00dc, &XMLTree::_paramqry },
  { 0x0013, &XMLTree::_password },
  { 0x004d, &XMLTree::_pls },
  { 0x000e, &XMLTree::_precision },
  { 0x002b, &XMLTree::_printgridlines },
  { 0x002a, &XMLTree::_printheaders },
  { 0x0012, &XMLTree::_protect },
  { 0x01af, &XMLTree::_prot4rev },
  { 0x01ad, &XMLTree::_qsi },
  { 0x00b9, &XMLTree::_recipname },
  { 0x000f, &XMLTree::_refmode },
  { 0x01b7, &XMLTree::_refreshall },
  { 0x0027, &XMLTree::_rightmargin },
  { 0x027e, &XMLTree::_rk },
  { 0x0208, &XMLTree::_row },
  { 0x00d6, &XMLTree::_rstring },
  { 0x005f, &XMLTree::_saverecalc },
  { 0x00af, &XMLTree::_scenario },
  { 0x00ae, &XMLTree::_scenman },
  { 0x00dd, &XMLTree::_scenprotect },
  { 0x00a0, &XMLTree::_scl },
  { 0x001d, &XMLTree::_selection },
  { 0x00a1, &XMLTree::_setup },
  { 0x00bc, &XMLTree::_shrfmla },
  { 0x0090, &XMLTree::_sort },
  { 0x0096, &XMLTree::_sound },
  { 0x00fc, &XMLTree::_sst },
  { 0x0099, &XMLTree::_standardwidth },
  { 0x0207, &XMLTree::_string },
  { 0x0293, &XMLTree::_style },
  { 0x0091, &XMLTree::_sub },
  { 0x01ae, &XMLTree::_supbook },
  { 0x00c6, &XMLTree::_sxdb },
  { 0x0122, &XMLTree::_sxdbex },
  { 0x00c5, &XMLTree::_sxdi },
  { 0x00f1, &XMLTree::_sxex },
  { 0x00dc, &XMLTree::_sxext },
  { 0x01bb, &XMLTree::_sxfdbtype },
  { 0x00f2, &XMLTree::_sxfilt },
  { 0x00fb, &XMLTree::_sxformat },
  { 0x0103, &XMLTree::_sxformula },
  { 0x00f9, &XMLTree::_sxfmla },
  { 0x00d5, &XMLTree::_sxidstm },
  { 0x00b4, &XMLTree::_sxivd },
  { 0x00b5, &XMLTree::_sxli },
  { 0x00f6, &XMLTree::_sxname },
  { 0x00f8, &XMLTree::_sxpair },
  { 0x00b6, &XMLTree::_sxpi },
  { 0x00f0, &XMLTree::_sxrule },
  { 0x00cd, &XMLTree::_sxstring },
  { 0x00f7, &XMLTree::_sxselect },
  { 0x00d0, &XMLTree::_sxtbl },
  { 0x00d2, &XMLTree::_sxtbpg },
  { 0x00d1, &XMLTree::_sxtbrgiitm },
  { 0x00b1, &XMLTree::_sxvd },
  { 0x0100, &XMLTree::_sxvdex },
  { 0x00b2, &XMLTree::_sxvi },
  { 0x00b0, &XMLTree::_sxview },
  { 0x00e3, &XMLTree::_sxvs },
  { 0x013d, &XMLTree::_tabid },
  { 0x00ea, &XMLTree::_tabidconf },
  { 0x0236, &XMLTree::_table },
  { 0x0060, &XMLTree::_templt },
  { 0x0028, &XMLTree::_topmargin },
  { 0x01b6, &XMLTree::_txo },
  { 0x00df, &XMLTree::_uddesc },
  { 0x005e, &XMLTree::_uncalced },
  { 0x01a9, &XMLTree::_userbview },
  { 0x01aa, &XMLTree::_usersviewbegin },
  { 0x01ab, &XMLTree::_usersviewend },
  { 0x0160, &XMLTree::_useselfs },
  { 0x0084, &XMLTree::_vcenter },
  { 0x001a, &XMLTree::_verticalpagebreaks },
  { 0x003d, &XMLTree::_window1 },
  { 0x023e, &XMLTree::_window2 },
  { 0x0019, &XMLTree::_windowprotect }, 
  { 0x005C, &XMLTree::_writeaccess },
  { 0x0086, &XMLTree::_writeprot },
  { 0x0081, &XMLTree::_wsbool },
  { 0x0059, &XMLTree::_xct },
  { 0x00e0, &XMLTree::_xf },
  { 0x0162, &XMLTree::_xl5modify },
  { 0x0000, NULL }
};

#endif
