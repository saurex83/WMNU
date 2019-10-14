/******************************************************
 *
 * This file contains 8051/8052 useful bit definitions
 *
 * Copyright 2004-2016 IAR Systems. All rights reserved.
 *
 * $Revision: 12165 $
 *
 ******************************************************/

#ifndef BITDEF_H
#define BITDEF_H

/*==============================*/
/* Predefined SFR Bit Addresses */
/*==============================*/
  
#ifdef __IAR_SYSTEMS_ICC__

/*========PSW========*/
#define CY  PSW_bit.CY
#define AC  PSW_bit.AC
#define F0  PSW_bit.F0
#define RS1 PSW_bit.RS1
#define RS0 PSW_bit.RS0
#define OV  PSW_bit.OV
#define P   PSW_bit.P

/*========TCON=======*/
#define TF1 TCON_bit.TF1
#define TR1 TCON_bit.TR1
#define TF0 TCON_bit.TF0
#define TR0 TCON_bit.TR0
#define IE1 TCON_bit.IE1
#define IT1 TCON_bit.IT1
#define IE0 TCON_bit.IE0
#define IT0 TCON_bit.IT0

/*========IE=========*/
#define EA  IE_bit.EA
#define ET2 IE_bit.ET2
#define ES  IE_bit.ES 
#define ET1 IE_bit.ET1
#define EX1 IE_bit.EX1
#define ET0 IE_bit.ET0
#define EX0 IE_bit.EX0

/*========IP=========*/
#define PT2 IP_bit.PT2
#define PS  IP_bit.PS 
#define PT1 IP_bit.PT1
#define PX1 IP_bit.PX1
#define PT0 IP_bit.PT0
#define PX0 IP_bit.PX0

/*========P3=========*/
#define RD   P3_bit.RD
#define WR   P3_bit.WR
#define T1   P3_bit.T1
#define T0   P3_bit.T0
#define INT1 P3_bit.INT1
#define INT0 P3_bit.INT0
#define TXD  P3_bit.TxD
#define RXD  P3_bit.RxD

/*========SCON========*/
#define SM0 SCON_bit.SM0
#define SM1 SCON_bit.SM1
#define SM2 SCON_bit.SM2
#define REN SCON_bit.REN
#define TB8 SCON_bit.TB8
#define RB8 SCON_bit.RB8
#define TI  SCON_bit.TI
#define RI  SCON_bit.RI

/*========T2CON=======*/
#define TF2    T2CON_bit.TF2
#define EXF2   T2CON_bit.EXF2
#define RCLK   T2CON_bit.RCLK
#define TCLK   T2CON_bit.TCLK
#define EXEN2  T2CON_bit.EXEN2
#define TR2    T2CON_bit.TR2
#define C_T2   T2CON_bit.C_T2
#define CP_RL2 T2CON_bit.CP_RL2

#endif // __IAR_SYSTEMS_ICC__

#ifdef __IAR_SYSTEMS_ASM__

/*========TCON=======*/
#define TF1 TCON_TF1
#define TR1 TCON_TR1
#define TF0 TCON_TF0
#define TR0 TCON_TR0
#define IE1 TCON_IE1
#define IT1 TCON_IT1
#define IE0 TCON_IE0
#define IT0 TCON_IT0

/*========IE=========*/
#define EA  IE_EA
#define ET2 IE_ET2
#define ES  IE_ES 
#define ET1 IE_ET1
#define EX1 IE_EX1
#define ET0 IE_ET0
#define EX0 IE_EX0

/*========IP=========*/
#define PT2 IP_PT2
#define PS  IP_PS 
#define PT1 IP_PT1
#define PX1 IP_PX1
#define PT0 IP_PT0
#define PX0 IP_PX0

/*========P3=========*/
#define RD   P3_RD
#define WR   P3_WR
#define T1   P3_T1
#define T0   P3_T0
#define INT1 P3_INT1
#define INT0 P3_INT0
#define TXD  P3_TxD
#define RXD  P3_RxD

/*========SCON========*/
#define SM0 SCON_SM0
#define SM1 SCON_SM1
#define SM2 SCON_SM2
#define REN SCON_REN
#define TB8 SCON_TB8
#define RB8 SCON_RB8
#define TI  SCON_TI
#define RI  SCON_RI

/*========T2CON=======*/
#define TF2    T2CON_TF2
#define EXF2   T2CON_EXF2
#define RCLK   T2CON_RCLK
#define TCLK   T2CON_TCLK
#define EXEN2  T2CON_EXEN2
#define TR2    T2CON_TR2
#define C_T2   T2CON_C_T2
#define CP_RL2 T2CON_CP_RL2

#endif // __IAR_SYSTEMS_ASM__

#endif // BITDEF_H
