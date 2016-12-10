













#ifndef LANGUAGE_H
#define LANGUAGE_H

#include "platform.h"
#include "gui/richtext.h"

extern std::string g_lang;

#define STR_SELFLANG		0
#define STR_IDCODE			1
#define STR_NEWGAME			2
#define STR_LOADGAME		3
#define STR_JOINGAME		4
#define STR_HOSTGAME		5
#define STR_OPTIONS			6
#define STR_QUIT			7
#define STR_WINDOWED		8
#define STR_FULLSCREEN		9
#define STR_DONE			10
#define STR_TUTORIAL		11
#define STR_TITLE			12
#define STR_MAPED			13
#define STR_VOLUMEPCT		14
#define STR_LANONLYNOTPUB	15
#define STR_NOMATCHSVNOT	16
#define STR_CONMATCHSVLIST	17
#define STR_CONMATCHLISTING	18
#define STR_CONTOMATCH		19
#define STR_CONMATCHGETNEXT	20
#define STR_CONMATCHREQSV	21
#define STR_GETSVLIST		22
#define STR_ADD				23
#define STR_CLEAR			24
#define STR_QREF			25
#define STR_REFRESH			26
#define STR_LANREF			27
#define STR_ADDRESS			28
#define STR_PORT			29

#define STR_LABOURER		30
#define STR_BATTLECOMP		31
#define STR_TRUCK			32
#define STR_CARLYLE			33

#define STR_HOUSE1			34
#define STR_STORE			35
#define STR_TRFAC			36
#define STR_FARM			37
#define STR_SHMINE			38
#define STR_IRONSM			39
#define STR_OILWELL			40
#define STR_OILREF			41
#define STR_NUCPOW			42
#define STR_COALPOW			43
#define STR_CEMPL			44
#define STR_CHEMPL			45
#define STR_ELECPL			46
#define STR_GASSTN			47

#define STR_ROAD			48
#define STR_POWL			49
#define STR_CRPIPE			50

#define STR_LOWERRESREC		51

#define STR_HOUSEDESC		52
#define STR_FACDESC			53
#define STR_OILREFDESC		54
#define STR_COALPOWDESC		55
#define STR_CHEMPLDESC		56
#define STR_GASSTNDESC		57
#define STR_CEMPLDESC		58
#define STR_IRONSMDESC		59
#define STR_NUCPOWDESC		60
#define STR_FARMDESC		61
#define STR_STOREDESC		62
#define STR_OILWELLDESC		63
#define STR_SHMINEDESC		64

#define STR_ROADDESC		65
#define STR_POWLDESC		66
#define STR_CRPIPEDESC		67

#define STR_RESFUNDS		68
#define STR_RESLABOUR		69
#define STR_RESHOUSING		70
#define STR_RESFARMPRODS	71
#define STR_RESPROD			72
#define STR_RESRETFOOD		73
#define STR_RESCROIL		74
#define STR_RESWSFUEL		75
#define STR_RESRETFUEL		76
#define STR_RESENERGY		77
#define STR_RESCHEMS		78
#define STR_RESIRONORE		79
#define STR_RESMETAL		80
#define STR_RESSTONE		81
#define STR_RESCEMENT		82
#define STR_RESCOAL			83
#define STR_RESURAN			84

#define STR_MEASDOLLARS		85
#define STR_MEASLABSECS		86
#define STR_MEASOCC			87
#define STR_MEASGRAMS		88
#define STR_MEASGALLONS		89
#define STR_MEASKWATTS		90
#define STR_MEASLITERS		91
#define STR_MEASKGRAMS		92
#define STR_MEASBUSHELS		93

#define STR_ADDSVDESC		94
#define STR_CLEARSVDESC		95
#define STR_REFSVDESC		96
#define STR_GETSVDESC		97
#define STR_SRLANDESC		98

#define STR_EARNPROD		99
#define STR_EXPCON			100

#define STR_CLOSE			101

#define STR_VERSION			102

#define STR_NEWMAP			103
#define STR_ELEVATION		104
#define STR_TERRAIN			105
#define STR_FOLIAGE			106
#define STR_UNITS			107
#define STR_BUILDINGS		108
#define STR_CONDUITS		109
#define STR_RESOURCES		110
#define STR_TRIGGERS		111

#define STR_PLACE			112
#define STR_DELETE			113
    
#define STR_RAISE			114
#define STR_LOWER			115
#define STR_SPREAD			116

#define STR_LOADMAP			117
#define STR_SAVEMAP			118
#define STR_QSAVE			119

#define STR_SAVE			120
#define STR_SAVETOFILE		121
#define STR_DELTHESAVE		122
#define STR_SHOWDEBUG		123
#define STR_SHOWTRANSX		124
#define STR_SAVEGAME2		125
#define STR_LOADGAME2		126
#define STR_PAUSEUPD		127
#define STR_PLAYUNLOCK		128
#define STR_FASTUNLOCK		129
#define STR_TRUCKMGR		130
#define STR_QUITTOMENU		131
#define STR_CANCEL			132
#define STR_CONTINUE		133
#define STR_SET				134
#define STR_GRAPHS			135
#define STR_STOCKLIST		136
#define STR_NONE			137
#define STR_BUY				138
#define STR_PRODLEVEL		139
#define STR_WAGE			140
#define STR_PRICEOF			141
#define STR_COST			142
#define STR_NEED			143
#define STR_HAVE			144
#define STR_TOORDERA		145
#define STR_INSUFFICIENT	146
#define STR_ATMANUF			147
#define STR_TOPRODUCE		148
#define STR_MANUFDA			149
#define STR_FOR				150
#define STR_ORDEREDA		151
#define STR_FROM			152
#define STR_SETPRICE		153
#define STR_AT				154
#define STR_TO				155
#define STR_SETWAGE			156
#define STR_SETCONWAGE		157
#define STR_SETTRANSPPR		158
#define STR_SETDRWAGE		159
#define STR_SETPRODLEV		160
#define STR_SETMANPR		161
#define STR_OF				162
#define STR_JOIN			163
#define STR_JOINCURGAME		164
#define STR_NOCONMATCH		165
#define STR_CONMATCH		166
#define STR_NOSV			167
#define STR_JOINING			168
#define STR_COPYRIGHT		169

#define STR_CONCOMP			170
#define STR_JOINEDGAME		171
#define STR_LEFTGAME		172
#define STR_HOSTDISC		173
#define STR_PLACEDA			174
#define STR_PAUSED			175
#define STR_PRESSEDPLAY		176
#define STR_PRESSEDFAST		177
#define STR_ERGAMEST		178
#define STR_ERMAPMATCH		179
#define STR_ERLOADMAP		180
#define STR_ERSVFULL		181
#define STR_ERVERMATCH		182
#define STR_TIMEDOUT1		183
#define STR_WASKICKED		184

#define STR_SECOND			185
#define STR_PLAYER			186
#define STR_NETWORTH		187
#define STR_UTILPROV		188
#define STR_THRUPUT			189
#define STR_TRANSPCOST		190
#define STR_DRWAGE			191

#define STR_LOAD			192
#define STR_LOADSELGAME		193
#define STR_DELSELGAME		194

#define STR_MAKEPUB			195
#define STR_CREATE			196
#define STR_GAMEROOM		197
#define STR_GAMENAME		198
#define STR_MAP				199

#define STR_CONREQS			200
#define STR_INPUTS			201
#define STR_OUTPUTS			202

#define STR_EVICTION		203
#define STR_STARVATION		204
#define STR_POPULATION		205
#define STR_GROWTH			206

#define STR_CONWAGE			207

#define STR_READY			208
#define STR_QUIT3			209
#define STR_TOGGLEREADY		210
#define STR_EXITSESS		211

#define STR_TILEWX			212
#define STR_TILEWY			213
#define STR_RANDOM			214
#define STR_RANDSEED		215
#define STR_GENERATE		216

#define STR_STARTINGIN		217

#define	STR_REST			218

#define STR_IDLE			219
#define STR_TOJOB			220
#define STR_ATJOB			221
#define STR_CSTR			222
#define STR_TRUCKING		223
#define STR_TOSUP			224
#define STR_TODEM			225
#define STR_TOREF			226
#define STR_REFUEL			227
#define STR_OFFLOAD			228
#define STR_LOADUP			229

#define STR_TOSTORE			230
#define STR_ATSTORE			231
#define STR_TOHOME			232
#define STR_ATHOME			233

#define STR_ESC				234
#define STR_RANKS			235

#define STR_BORDERS			236

#define STR_BOUGHT			237
#define STR_PUTUP			238
#define STR_FORSALEFOR		239
#define STR_TOOKDOWN		240
#define STR_FROMSALE		241
#define STR_CHANGEDPROPPRICEOF	242

#define STR_PROPERTYPRICE	243
#define STR_BUYPROP			244
#define STR_BUYTHISBL		245
#define STR_FORSALE			246

#define STR_DONTHAVETR		247

#define STR_PLAYSLOTON		248
#define STR_ISAIPLAYER		249
#define STR_PARENTSTATE		250
#define STR_INSTTYPE		251
#define STR_MUSTBESINGLE	252
#define STR_STILLDOWN		253
#define STR_MAPNOTLOADED	254
#define STR_ERRLOADM		255
#define STR_TIMEDOUT		256
#define STR_LEFT			257
#define STR_OVERMAP			258
#define STR_INSUFBUY		259

#define STR_WIDTHMUSTBE		260
#define STR_AND				261

#define STR_INOUT			262
#define STR_DEM				263

#define STR_DATAUNAV		264

#define STR_DEMSUPAVC		265

#define STR_SUP				266

#define STR_AVC				267

#define STR_DEMOL			268
#define STR_DEMOLD			269

#define STR_MGC				270

#define STR_PLACED			271

#define STR_AVPS			272
#define STR_AVPE			273
#define STR_TOTPS			274
#define STR_TOTPE			275

#define STR_BANK			276
#define STR_BANKDESC		277

#define STR_CVNOTE			278

#define STR_MEASPROD		279

#define STR_T				280
#define STR_T0				281
#define STR_T1				282
#define STR_T2				283
#define STR_T3				284
#define STR_T4				285
#define STR_T5				286
#define STR_T6				287
#define STR_T7				288
#define STR_T8				289
#define STR_T9				290
#define STR_T10				291
#define STR_T11				292
#define STR_T12				293
#define STR_T13				294
#define STR_T14				295
#define STR_T15				296
#define STR_T16				297
#define STR_T17				298
#define STR_T18				299
#define STR_T19				300
#define STR_T20				301
#define STR_T21				302
#define STR_T22				303
#define STR_T23				304
#define STR_T24				305
#define STR_T25				306
#define STR_T26				307
#define STR_T27				308
#define STR_T28				309
#define STR_T29				310
#define STR_T30				311

#define STR_TYPES			312

extern RichText STRTABLE[STR_TYPES];
extern const char* STRFILE[STR_TYPES];

ecbool SwitchLang(const char* dir);
std::string LoadLangStr(const char* lang, const char* file);

#endif