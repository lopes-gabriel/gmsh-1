
//Mesh.CharacteristicLengthFactor = 0.7; // 32,000 tets
//Mesh.CharacteristicLengthFactor = 0.5; // 91,000 tets
//Mesh.CharacteristicLengthFactor = 0.4; // 163,000 tets
//Mesh.CharacteristicLengthFactor = 0.3; // 340,000 tets

R1 = 1.e-3;
R2 = 1.5e-3;

h1 = 0.7e-3;
h2 = 0.1e-3;
h3 = 1.e-3;
h4 = 2.01e-3;

w1 = 1.1e-3;
w2 = 3.005e-3;

hm = 5e-3;
Rm = 7e-3;

w = 0.5e-3;

p0 = 1.5e-3; // center coils
p3 = 3e-3;
p7 = 0.5e-3; // coils inside
p8 = 0.6e-3; // coils outside
p9 = 0.3e-3;

Point(101) = {0,0,h1,p0};
Point(102) = {+R1,0,h1,p7};
Point(103) = {+R2,0,h1,p8};
Point(104) = {-R1,0,h1,p7};
Point(105) = {-R2,0,h1,p8};
Point(106) = {0,R1,h1,p7};
Point(107) = {0,-R1,h1,p7};
Point(108) = {0,R2,h1,p8};
Point(109) = {0,-R2,h1,p8};

Point(201) = {0,0,-h1,p0};
Point(202) = {+R1,0,-h1,p7};
Point(203) = {+R2,0,-h1,p8};
Point(204) = {-R1,0,-h1,p7};
Point(205) = {-R2,0,-h1,p8};
Point(206) = {0,R1,-h1,p7};
Point(207) = {0,-R1,-h1,p7};
Point(208) = {0,R2,-h1,p8};
Point(209) = {0,-R2,-h1,p8};

Point(1101) = {0,0,h2,p0};
Point(1102) = {+R1,0,h2,p7};
Point(1103) = {+R2,0,h2,p8};
Point(1104) = {-R1,0,h2,p7};
Point(1105) = {-R2,0,h2,p8};
Point(1106) = {0,R1,h2,p7};
Point(1107) = {0,-R1,h2,p7};
Point(1108) = {0,R2,h2,p8};
Point(1109) = {0,-R2,h2,p8};

Point(1201) = {0,0,-h2,p0};
Point(1202) = {+R1,0,-h2,p7};
Point(1203) = {+R2,0,-h2,p8};
Point(1204) = {-R1,0,-h2,p7};
Point(1205) = {-R2,0,-h2,p8};
Point(1206) = {0,R1,-h2,p7};
Point(1207) = {0,-R1,-h2,p7};
Point(1208) = {0,R2,-h2,p8};
Point(1209) = {0,-R2,-h2,p8};

Circle(1) = {102,101,106};
Circle(2) = {106,101,104};
Circle(3) = {104,101,107};
Circle(4) = {107,101,102};
Circle(5) = {103,101,108};
Circle(6) = {108,101,105};
Circle(7) = {105,101,109};
Circle(8) = {109,101,103};
Circle(9) = {202,201,206};
Circle(10) = {206,201,204};
Circle(11) = {204,201,207};
Circle(12) = {207,201,202};
Circle(13) = {203,201,208};
Circle(14) = {208,201,205};
Circle(15) = {205,201,209};
Circle(16) = {209,201,203};

Circle(101) = {1102,1101,1106};
Circle(102) = {1106,1101,1104};
Circle(103) = {1104,1101,1107};
Circle(104) = {1107,1101,1102};
Circle(105) = {1103,1101,1108};
Circle(106) = {1108,1101,1105};
Circle(107) = {1105,1101,1109};
Circle(108) = {1109,1101,1103};
Circle(109) = {1202,1201,1206};
Circle(1010) = {1206,1201,1204};
Circle(1011) = {1204,1201,1207};
Circle(1012) = {1207,1201,1202};
Circle(1013) = {1203,1201,1208};
Circle(1014) = {1208,1201,1205};
Circle(1015) = {1205,1201,1209};
Circle(1016) = {1209,1201,1203};

Point(2101) = {0,0,hm,p3};
Point(2102) = {+Rm,0,hm,p3};
Point(2104) = {-Rm,0,hm,p3};
Point(2106) = {0,Rm,hm,p3};
Point(2107) = {0,-Rm,hm,p3};
Point(2201) = {0,0,-hm,p3};
Point(2202) = {+Rm,0,-hm,p3};
Point(2204) = {-Rm,0,-hm,p3};
Point(2206) = {0,Rm,-hm,p3};
Point(2207) = {0,-Rm,-hm,p3};

Point(3202) = {+Rm,0,h2,p3};
Point(3204) = {-Rm,0,h2,p3};
Point(3206) = {0,Rm,h2,p3};
Point(3207) = {0,-Rm,h2,p3};
Point(3302) = {+Rm,0,-h2,p3};
Point(3304) = {-Rm,0,-h2,p3};
Point(3306) = {0,Rm,-h2,p3};
Point(3307) = {0,-Rm,-h2,p3};

Line(1017) = {203,1203};
Line(1018) = {1203,1103};
Line(1019) = {1103,103};
Line(1020) = {208,1208};
Line(1021) = {1208,1108};
Line(1022) = {1108,108};
Line(1023) = {209,1209};
Line(1024) = {1209,1109};
Line(1025) = {1109,109};
Line(1026) = {205,1205};
Line(1027) = {1205,1105};
Line(1028) = {1105,105};
Line(1029) = {207,1207};
Line(1030) = {1207,1107};
Line(1031) = {1107,107};
Line(1032) = {202,1202};
Line(1033) = {1202,1102};
Line(1034) = {1102,102};
Line(1035) = {206,1206};
Line(1036) = {1206,1106};
Line(1037) = {1106,106};
Line(1038) = {204,1204};
Line(1039) = {1204,1104};
Line(1040) = {1104,104};

Circle(1041) = {2102,2101,2106};
Circle(1042) = {2106,2101,2104};
Circle(1043) = {2104,2101,2107};
Circle(1044) = {2107,2101,2102};

Circle(1045) = {3206,1101,3204};
Circle(1046) = {3204,1101,3207};
Circle(1047) = {3207,1101,3202};
Circle(1048) = {3202,1101,3206};
Circle(1049) = {3306,1201,3302};
Circle(1050) = {3302,1201,3307};

Circle(1051) = {3307,1201,3304};
Circle(1052) = {3304,1201,3306};
Circle(1053) = {2204,2201,2207};
Circle(1054) = {2207,2201,2202};
Circle(1055) = {2202,2201,2206};
Circle(1056) = {2206,2201,2204};

Line(1057) = {2206,3306};
Line(1058) = {3306,3206};
Line(1059) = {3206,2106};
Line(1060) = {2204,3304};
Line(1061) = {3304,3204};
Line(1062) = {3204,2104};
Line(1063) = {2207,3307};
Line(1064) = {3307,3207};
Line(1065) = {3207,2107};
Line(1066) = {2202,3302};
Line(1067) = {3302,3202};
Line(1068) = {3202,2102};

Line Loop(1069) = {1016,1013,1014,1015};
Line Loop(1070) = {109,1010,1011,1012};
Plane Surface(1071) = {1069,1070};
Line Loop(1072) = {106,107,108,105};
Line Loop(1073) = {103,104,101,102};
Plane Surface(1074) = {1072,1073};
Line Loop(1081) = {5,6,7,8};
Line Loop(1082) = {3,4,1,2};
Plane Surface(1083) = {1081,1082};
Line Loop(1085) = {16,13,14,15};
Line Loop(1086) = {9,10,11,12};
Plane Surface(1087) = {1085,1086};
Line Loop(1089) = {1041,1042,1043,1044};
Plane Surface(1090) = {1089};
Line Loop(1091) = {1055,1056,1053,1054};
Plane Surface(1092) = {1091};
Line Loop(1093) = {-1062,-1045,1059,1042};
Ruled Surface(1094) = {1093};
Line Loop(1095) = {-1057,1056,1060,1052};
Ruled Surface(1096) = {1095};
Line Loop(1097) = {-1045,-1058,-1052,1061};
Ruled Surface(1098) = {1097};
Line Loop(1099) = {1059,-1041,-1068,1048};
Ruled Surface(1100) = {1099};
Line Loop(1101) = {-1048,-1067,-1049,1058};
Ruled Surface(1102) = {1101};
Line Loop(1103) = {-1066,1055,1057,1049};
Ruled Surface(1104) = {1103};
Line Loop(1105) = {1068,-1044,-1065,1047};
Ruled Surface(1106) = {1105};
Line Loop(1107) = {-1047,-1064,-1050,1067};
Ruled Surface(1108) = {1107};
Line Loop(1109) = {1050,-1063,1054,1066};
Ruled Surface(1110) = {1109};
Line Loop(1111) = {-1043,-1062,1046,1065};
Ruled Surface(1112) = {1111};
Line Loop(1113) = {-1046,-1061,-1051,1064};
Ruled Surface(1114) = {1113};
Line Loop(1115) = {1063,1051,-1060,1053};
Ruled Surface(1116) = {1115};
Line Loop(1117) = {1022,-5,-1019,105};
Ruled Surface(1118) = {1117};
Line Loop(1119) = {8,-1019,-108,1025};
Ruled Surface(1120) = {1119};
Line Loop(1121) = {-7,-1028,107,1025};
Ruled Surface(1122) = {1121};
Line Loop(1123) = {-6,-1022,106,1028};
Ruled Surface(1124) = {1123};
Line Loop(1125) = {108,-1018,-1016,1024};
Ruled Surface(1126) = {1125};
Line Loop(1127) = {-107,-1027,1015,1024};
Ruled Surface(1128) = {1127};
Line Loop(1129) = {106,-1027,-1014,1021};
Ruled Surface(1130) = {1129};
Line Loop(1131) = {105,-1021,-1013,1018};
Ruled Surface(1132) = {1131};
Line Loop(1133) = {1017,-1016,-1023,16};
Ruled Surface(1134) = {1133};
Line Loop(1135) = {-1013,-1017,13,1020};
Ruled Surface(1136) = {1135};
Line Loop(1137) = {-1014,-1020,14,1026};
Ruled Surface(1138) = {1137};
Line Loop(1139) = {-1023,-15,1026,1015};
Ruled Surface(1140) = {1139};
Line Loop(1141) = {-1010,-1035,10,1038};
Ruled Surface(1142) = {1141};
Line Loop(1143) = {-109,-1032,9,1035};
Ruled Surface(1144) = {1143};
Line Loop(1145) = {-1012,-1029,12,1032};
Ruled Surface(1146) = {1145};
Line Loop(1147) = {1029,-1011,-1038,11};
Ruled Surface(1148) = {1147};
Line Loop(1149) = {-104,-1030,1012,1033};
Ruled Surface(1150) = {1149};
Line Loop(1151) = {-101,-1033,109,1036};
Ruled Surface(1152) = {1151};
Line Loop(1153) = {102,-1039,-1010,1036};
Ruled Surface(1154) = {1153};
Line Loop(1155) = {1030,-103,-1039,1011};
Ruled Surface(1156) = {1155};
Line Loop(1157) = {4,-1034,-104,1031};
Ruled Surface(1158) = {1157};
Line Loop(1159) = {3,-1031,-103,1040};
Ruled Surface(1160) = {1159};
Line Loop(1161) = {-2,-1037,102,1040};
Ruled Surface(1162) = {1161};
Line Loop(1163) = {-1,-1034,101,1037};
Ruled Surface(1164) = {1163};

Point(10001) = {w,w,h1,p9};
Point(10002) = {w,-w,h1,p9};
Point(10003) = {-w,w,h1,p9};
Point(10004) = {-w,-w,h1,p9};

Point(10011) = {w,w,h2,p9};
Point(10012) = {w,-w,h2,p9};
Point(10013) = {-w,w,h2,p9};
Point(10014) = {-w,-w,h2,p9};

Point(20001) = {w,w,-h1,p9};
Point(20002) = {w,-w,-h1,p9};
Point(20003) = {-w,w,-h1,p9};
Point(20004) = {-w,-w,-h1,p9};

Point(20011) = {w,w,-h2,p9};
Point(20012) = {w,-w,-h2,p9};
Point(20013) = {-w,w,-h2,p9};
Point(20014) = {-w,-w,-h2,p9};

Line(1173) = {10004,10003};
Line(1174) = {10003,10001};
Line(1175) = {10001,10002};
Line(1176) = {10002,10004};
Line(1177) = {10014,10013};
Line(1178) = {10013,10011};
Line(1179) = {10011,10012};
Line(1180) = {10012,10014};
Line(1181) = {20014,20013};
Line(1182) = {20013,20011};
Line(1183) = {20011,20012};
Line(1184) = {20012,20014};
Line(1185) = {20004,20003};
Line(1186) = {20003,20001};
Line(1187) = {20001,20002};
Line(1188) = {20002,20004};
Line(1189) = {20004,20014};
Line(1190) = {20014,10014};
Line(1191) = {10014,10004};
Line(1192) = {20002,20012};
Line(1193) = {20012,10012};
Line(1194) = {10012,10002};
Line(1195) = {20001,20011};
Line(1196) = {20011,10011};
Line(1197) = {10011,10001};
Line(1198) = {20003,20013};
Line(1199) = {20013,10013};
Line(1200) = {10013,10003};

Line Loop(1201) = {1176,1173,1174,1175};
Plane Surface(1202) = {1082,1201};
Line Loop(1203) = {1180,1177,1178,1179};
Plane Surface(1204) = {1073,1203};
Line Loop(1205) = {1181,1182,1183,1184};
Plane Surface(1206) = {1070,1205};
Line Loop(1207) = {1185,1186,1187,1188};
Plane Surface(1208) = {1086,1207};
Plane Surface(1209) = {1207};
Plane Surface(1210) = {1205};
Plane Surface(1211) = {1203};
Plane Surface(1212) = {1201};
Line Loop(1213) = {1194,-1175,-1197,1179};
Plane Surface(1214) = {1213};
Line Loop(1215) = {1193,-1179,-1196,1183};
Plane Surface(1216) = {1215};
Line Loop(1217) = {1192,-1183,-1195,1187};
Plane Surface(1218) = {1217};
Line Loop(1219) = {1189,-1184,-1192,1188};
Plane Surface(1220) = {1219};
Line Loop(1221) = {1190,-1180,-1193,1184};
Plane Surface(1222) = {1221};
Line Loop(1223) = {1191,-1176,-1194,1180};
Plane Surface(1224) = {1223};
Line Loop(1225) = {-1181,-1189,1185,1198};
Plane Surface(1226) = {1225};
Line Loop(1227) = {1199,-1177,-1190,1181};
Plane Surface(1228) = {1227};
Line Loop(1229) = {1200,-1173,-1191,1177};
Plane Surface(1230) = {1229};
Line Loop(1231) = {1174,-1197,-1178,1200};
Plane Surface(1232) = {1231};
Line Loop(1233) = {1178,-1196,-1182,1199};
Plane Surface(1234) = {1233};
Line Loop(1235) = {-1195,-1186,1198,1182};
Plane Surface(1236) = {1235};

Point(30001) = {w,w,h3,p9};
Point(30002) = {w,-w,h3,p9};
Point(30003) = {-w,w,h3,p9};
Point(30004) = {-w,-w,h3,p9};
Point(30005) = {w,w,-h3,p9};
Point(30006) = {w,-w,-h3,p9};
Point(30007) = {-w,w,-h3,p9};
Point(30008) = {-w,-w,-h3,p9};

Point(30011) = {w1,w,h3,p9};
Point(30012) = {w1,-w,h3,p9};
Point(30013) = {-w,w,h4,p9};
Point(30014) = {-w,-w,h4,p9};
Point(30021) = {w,w,h4,p9};
Point(30022) = {w,-w,h4,p9};

Point(30211) = {w1,w,-h3,p9};
Point(30212) = {w1,-w,-h3,p9};
Point(30213) = {-w,w,-h4,p9};
Point(30214) = {-w,-w,-h4,p9};
Point(30221) = {w,w,-h4,p9};
Point(30222) = {w,-w,-h4,p9};

Point(30311) = {w2-w1,w,h3,p9};
Point(30312) = {w2-w1,-w,h3,p9};
Point(30313) = {w2+w,w,h4,p9};
Point(30314) = {w2+w,-w,h4,p9};
Point(30321) = {w2-w,w,h4,p9};
Point(30322) = {w2-w,-w,h4,p9};
Point(30401) = {w2-w,w,h3,p9};
Point(30402) = {w2-w,-w,h3,p9};
Point(30403) = {w2+w,w,h3,p9};
Point(30404) = {w2+w,-w,h3,p9};
Point(30441) = {w2-w,w,h1,p9};
Point(30442) = {w2-w,-w,h1,p9};

Point(40311) = {w2-w1,w,-h3,p9};
Point(40312) = {w2-w1,-w,-h3,p9};
Point(40313) = {w2+w,w,-h4,p9};
Point(40314) = {w2+w,-w,-h4,p9};
Point(40321) = {w2-w,w,-h4,p9};
Point(40322) = {w2-w,-w,-h4,p9};
Point(40401) = {w2-w,w,-h3,p9};
Point(40402) = {w2-w,-w,-h3,p9};
Point(40403) = {w2+w,w,-h3,p9};
Point(40404) = {w2+w,-w,-h3,p9};
Point(40441) = {w2-w,w,-h1,p9};
Point(40442) = {w2-w,-w,-h1,p9};

Line(1255) = {10004,30004};
Line(1256) = {30004,30014};
Line(1257) = {30014,30022};
Line(1258) = {30022,30322};
Line(1259) = {30322,30314};
Line(1260) = {30314,30404};
Line(1261) = {10002,30002};
Line(1262) = {30002,30012};
Line(1263) = {30012,30312};
Line(1264) = {30312,30402};
Line(1265) = {30402,30442};
Line(1266) = {10003,30003};
Line(1267) = {30003,30013};
Line(1268) = {30013,30021};
Line(1269) = {30021,30321};
Line(1270) = {30321,30313};
Line(1271) = {30313,30403};
Line(1272) = {10001,30001};
Line(1273) = {30001,30011};
Line(1274) = {30011,30311};
Line(1275) = {30311,30401};
Line(1276) = {30401,30441};
Line(1277) = {30014,30013};
Line(1279) = {30002,30001};
Line(1282) = {30402,30401};
Line(1285) = {30314,30313};

Point(30541) = {w2-w,w,h2,p9};
Point(30542) = {w2-w,-w,h2,p9};
Point(30603) = {w2+w,w,h2,p9};
Point(30604) = {w2+w,-w,h2,p9};
Point(40541) = {w2-w,w,-h2,p9};
Point(40542) = {w2-w,-w,-h2,p9};
Point(40603) = {w2+w,w,-h2,p9};
Point(40604) = {w2+w,-w,-h2,p9};

Line(1287) = {30442,30542};
Line(1288) = {30441,30541};
Line(1289) = {30541,30542};
Line(1290) = {30404,30604};
Line(1291) = {30403,30603};
Line(1292) = {30603,30604};
Line(1293) = {30604,30542};
Line(1294) = {30541,30603};
Line(1295) = {30542,40542};
Line(1296) = {40542,40442};
Line(1297) = {40442,40402};
Line(1298) = {30541,40541};
Line(1299) = {40541,40441};
Line(1300) = {40441,40401};
Line(1301) = {30604,40604};
Line(1302) = {40604,40404};
Line(1303) = {30603,40603};
Line(1304) = {40603,40403};
Line(1305) = {40402,40312};
Line(1306) = {40312,30212};
Line(1307) = {30212,30006};
Line(1308) = {30006,20002};
Line(1309) = {40403,40313};
Line(1310) = {40313,40321};
Line(1311) = {40321,30221};
Line(1312) = {30213,30221};
Line(1313) = {40404,40314};
Line(1314) = {40314,40322};
Line(1315) = {40322,30222};
Line(1316) = {30222,30214};
Line(1317) = {30214,30008};
Line(1318) = {30213,30007};
Line(1319) = {30213,30214};
Line(1320) = {40313,40314};
Line(1321) = {40401,40311};
Line(1322) = {40311,30211};
Line(1323) = {30211,30005};
Line(1324) = {30005,20001};
Line(1325) = {20003,30007};
Line(1326) = {20004,30008};
Line(1329) = {40604,40542};
Line(1330) = {40542,40541};
Line(1331) = {40541,40603};
Line(1332) = {40603,40604};

Line Loop(1333) = {1263,1264,1265,1287,-1293,-1290,-1260,-1259,-1258,-1257,-1256,-1255,-1176,1261,1262};
Plane Surface(1334) = {1333};
Line Loop(1335) = {1268,1269,1270,1271,1291,-1294,-1288,-1276,-1275,-1274,-1273,-1272,-1174,1266,1267};
Plane Surface(1336) = {1335};
Line Loop(1337) = {1266,1267,-1277,-1256,-1255,1173};
Plane Surface(1338) = {1337};
Line Loop(1339) = {1279,-1272,1175,1261};
Plane Surface(1340) = {1339};
Line Loop(1341) = {1273,1274,1275,-1282,-1264,-1263,-1262,1279};
Plane Surface(1342) = {1341};
Line Loop(1343) = {-1287,-1265,1282,1276,1288,1289};
Plane Surface(1344) = {1343};
Line Loop(1345) = {-1270,-1269,-1268,-1277,1257,1258,1259,1285};
Plane Surface(1346) = {1345};
Line Loop(1347) = {-1290,-1260,1285,1271,1291,1292};
Plane Surface(1348) = {1347};
Line Loop(1349) = {1329,1330,1331,1332};
Plane Surface(1350) = {1349};
Line Loop(1351) = {1293,-1289,1294,1292};
Plane Surface(1352) = {1351};
Line Loop(1353) = {1331,-1303,-1294,1298};
Plane Surface(1354) = {1353};
Line Loop(1355) = {-1330,-1295,-1289,1298};
Plane Surface(1356) = {1355};
Line Loop(1357) = {1329,-1295,-1293,1301};
Plane Surface(1358) = {1357};
Line Loop(1359) = {-1332,-1303,1292,1301};
Plane Surface(1360) = {1359};
Line Loop(1361) = {-1313,-1302,-1332,1304,1309,1320};
Plane Surface(1362) = {1361};
Line(1363) = {40401,40402};
Line(1364) = {30005,30006};
Line Loop(1365) = {-1297,-1296,1330,1299,1300,1363};
Plane Surface(1366) = {1365};
Line Loop(1367) = {1305,1306,1307,1308,1188,1326,-1317,-1316,-1315,-1314,-1313,-1302,1329,1296,1297};
Plane Surface(1368) = {1367};
Line Loop(1369) = {1321,1322,1323,1324,-1186,1325,-1318,1312,-1311,-1310,-1309,-1304,-1331,1299,1300};
Plane Surface(1370) = {1369};
Line Loop(1371) = {-1316,-1315,-1314,-1320,1310,1311,-1312,1319};
Plane Surface(1372) = {1371};
Line Loop(1373) = {1317,-1326,1185,1325,-1318,1319};
Plane Surface(1374) = {1373};
Line Loop(1375) = {-1187,-1324,1364,1308};
Plane Surface(1376) = {1375};
Line Loop(1377) = {1305,1306,1307,-1364,-1323,-1322,-1321,1363};
Plane Surface(1378) = {1377};

// coil top
Surface Loop(1379) = {1074,1124,1083,1118,1120,1122,1160,1158,1164,1162};
Volume(1380) = {1379};
// coil middle
Surface Loop(1381) = {1152,1074,1130,1128,1071,1126,1132,1154,1156,1150};
Volume(1382) = {1381};
// coil bottom
Surface Loop(1383) = {1071,1134,1136,1087,1138,1140,1144,1146,1148,1142};
Volume(1384) = {1383};

// magnet inside coil, top
Surface Loop(1385) = {1211,1224,1230,1232,1212,1214};
Volume(1386) = {1385};
// air gap
Surface Loop(1387) = {1210,1228,1234,1211,1222,1216};
Volume(1388) = {1387};
// magnet inside coil, bottom
Surface Loop(1389) = {1209,1226,1210,1236,1218,1220};
Volume(1390) = {1389};

// magnet outside coil, top
Surface Loop(100000) = {1352,1334,1342,1336,1346,1338,1212,1340,1348,1344};
Volume(100001) = {100000};
// magnet outside coil, middle
Surface Loop(100002) = {1350,1358,1356,1352,1354,1360};
Volume(100003) = {100002};
// magnet outside coil, bottom
Surface Loop(100004) = {1209,1374,1368,1378,1376,1370,1372,1362,1350,1366};
Volume(100005) = {100004};

// 2 missing cut surfaces
Line Loop(100006) = {1047,1048,1045,1046};
Plane Surface(100007) = {100006,1072,1351};
Line Loop(100008) = {1049,1050,1051,1052};
Plane Surface(100009) = {100008,1069,1349};

// cut volume, inside
Surface Loop(100010) = {1216,1222,1228,1234,1204,1156,1150,1206,1152,1154};
Volume(100011) = {100010};

// cut volume, outside
Surface Loop(100012) = {1356,100009,1102,100007,1108,1114,1098,1130,1128,1126,1132,1358,1360,1354};
Volume(100013) = {100012};

// air top
Surface Loop(100014) = {1202,1083,1118,1124,100007,1106,1100,1094,1112,1090,1122,1120,1334,1342,1336,1346,1338,1348,1344,1340};
Volume(100015) = {100014};

// air "hole" top (above inside cut)
Surface Loop(100016) = {1214,1224,1230,1232,1202,1160,1158,1164,1204,1162};
Volume(100017) = {100016};

// air "hole" bottom (below inside cut)
Surface Loop(100018) = {1236,1218,1220,1226,1206,1144,1146,1148,1142,1208};
Volume(100019) = {100018};

// air bottom
Surface Loop(100020) = {1138,100009,1104,1110,1116,1096,1092,1134,1136,1087,1140,1208,1374,1368,1378,1376,1370,1372,1362,1366};
Volume(100021) = {100020};
