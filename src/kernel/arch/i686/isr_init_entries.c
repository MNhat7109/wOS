#include <kernel/arch/i686/idt.h>
#include <kernel/arch/i686/gdt.h>

#define DEFINE_ISR(n) __attribute__((cdecl)) void isr_int##n(void);

DEFINE_ISR(0)
DEFINE_ISR(1)
DEFINE_ISR(2)
DEFINE_ISR(3)
DEFINE_ISR(4)
DEFINE_ISR(5)
DEFINE_ISR(6)
DEFINE_ISR(7)
DEFINE_ISR(8)
DEFINE_ISR(9)
DEFINE_ISR(10)
DEFINE_ISR(11)
DEFINE_ISR(12)
DEFINE_ISR(13)
DEFINE_ISR(14)
DEFINE_ISR(15)
DEFINE_ISR(16)
DEFINE_ISR(17)
DEFINE_ISR(18)
DEFINE_ISR(19)
DEFINE_ISR(20)
DEFINE_ISR(21)
DEFINE_ISR(22)
DEFINE_ISR(23)
DEFINE_ISR(24)
DEFINE_ISR(25)
DEFINE_ISR(26)
DEFINE_ISR(27)
DEFINE_ISR(28)
DEFINE_ISR(29)
DEFINE_ISR(30)
DEFINE_ISR(31)
DEFINE_ISR(32)
DEFINE_ISR(33)
DEFINE_ISR(34)
DEFINE_ISR(35)
DEFINE_ISR(36)
DEFINE_ISR(37)
DEFINE_ISR(38)
DEFINE_ISR(39)
DEFINE_ISR(40)
DEFINE_ISR(41)
DEFINE_ISR(42)
DEFINE_ISR(43)
DEFINE_ISR(44)
DEFINE_ISR(45)
DEFINE_ISR(46)
DEFINE_ISR(47)
DEFINE_ISR(48)
DEFINE_ISR(49)
DEFINE_ISR(50)
DEFINE_ISR(51)
DEFINE_ISR(52)
DEFINE_ISR(53)
DEFINE_ISR(54)
DEFINE_ISR(55)
DEFINE_ISR(56)
DEFINE_ISR(57)
DEFINE_ISR(58)
DEFINE_ISR(59)
DEFINE_ISR(60)
DEFINE_ISR(61)
DEFINE_ISR(62)
DEFINE_ISR(63)
DEFINE_ISR(64)
DEFINE_ISR(65)
DEFINE_ISR(66)
DEFINE_ISR(67)
DEFINE_ISR(68)
DEFINE_ISR(69)
DEFINE_ISR(70)
DEFINE_ISR(71)
DEFINE_ISR(72)
DEFINE_ISR(73)
DEFINE_ISR(74)
DEFINE_ISR(75)
DEFINE_ISR(76)
DEFINE_ISR(77)
DEFINE_ISR(78)
DEFINE_ISR(79)
DEFINE_ISR(80)
DEFINE_ISR(81)
DEFINE_ISR(82)
DEFINE_ISR(83)
DEFINE_ISR(84)
DEFINE_ISR(85)
DEFINE_ISR(86)
DEFINE_ISR(87)
DEFINE_ISR(88)
DEFINE_ISR(89)
DEFINE_ISR(90)
DEFINE_ISR(91)
DEFINE_ISR(92)
DEFINE_ISR(93)
DEFINE_ISR(94)
DEFINE_ISR(95)
DEFINE_ISR(96)
DEFINE_ISR(97)
DEFINE_ISR(98)
DEFINE_ISR(99)
DEFINE_ISR(100)
DEFINE_ISR(101)
DEFINE_ISR(102)
DEFINE_ISR(103)
DEFINE_ISR(104)
DEFINE_ISR(105)
DEFINE_ISR(106)
DEFINE_ISR(107)
DEFINE_ISR(108)
DEFINE_ISR(109)
DEFINE_ISR(110)
DEFINE_ISR(111)
DEFINE_ISR(112)
DEFINE_ISR(113)
DEFINE_ISR(114)
DEFINE_ISR(115)
DEFINE_ISR(116)
DEFINE_ISR(117)
DEFINE_ISR(118)
DEFINE_ISR(119)
DEFINE_ISR(120)
DEFINE_ISR(121)
DEFINE_ISR(122)
DEFINE_ISR(123)
DEFINE_ISR(124)
DEFINE_ISR(125)
DEFINE_ISR(126)
DEFINE_ISR(127)
DEFINE_ISR(128)
DEFINE_ISR(129)
DEFINE_ISR(130)
DEFINE_ISR(131)
DEFINE_ISR(132)
DEFINE_ISR(133)
DEFINE_ISR(134)
DEFINE_ISR(135)
DEFINE_ISR(136)
DEFINE_ISR(137)
DEFINE_ISR(138)
DEFINE_ISR(139)
DEFINE_ISR(140)
DEFINE_ISR(141)
DEFINE_ISR(142)
DEFINE_ISR(143)
DEFINE_ISR(144)
DEFINE_ISR(145)
DEFINE_ISR(146)
DEFINE_ISR(147)
DEFINE_ISR(148)
DEFINE_ISR(149)
DEFINE_ISR(150)
DEFINE_ISR(151)
DEFINE_ISR(152)
DEFINE_ISR(153)
DEFINE_ISR(154)
DEFINE_ISR(155)
DEFINE_ISR(156)
DEFINE_ISR(157)
DEFINE_ISR(158)
DEFINE_ISR(159)
DEFINE_ISR(160)
DEFINE_ISR(161)
DEFINE_ISR(162)
DEFINE_ISR(163)
DEFINE_ISR(164)
DEFINE_ISR(165)
DEFINE_ISR(166)
DEFINE_ISR(167)
DEFINE_ISR(168)
DEFINE_ISR(169)
DEFINE_ISR(170)
DEFINE_ISR(171)
DEFINE_ISR(172)
DEFINE_ISR(173)
DEFINE_ISR(174)
DEFINE_ISR(175)
DEFINE_ISR(176)
DEFINE_ISR(177)
DEFINE_ISR(178)
DEFINE_ISR(179)
DEFINE_ISR(180)
DEFINE_ISR(181)
DEFINE_ISR(182)
DEFINE_ISR(183)
DEFINE_ISR(184)
DEFINE_ISR(185)
DEFINE_ISR(186)
DEFINE_ISR(187)
DEFINE_ISR(188)
DEFINE_ISR(189)
DEFINE_ISR(190)
DEFINE_ISR(191)
DEFINE_ISR(192)
DEFINE_ISR(193)
DEFINE_ISR(194)
DEFINE_ISR(195)
DEFINE_ISR(196)
DEFINE_ISR(197)
DEFINE_ISR(198)
DEFINE_ISR(199)
DEFINE_ISR(200)
DEFINE_ISR(201)
DEFINE_ISR(202)
DEFINE_ISR(203)
DEFINE_ISR(204)
DEFINE_ISR(205)
DEFINE_ISR(206)
DEFINE_ISR(207)
DEFINE_ISR(208)
DEFINE_ISR(209)
DEFINE_ISR(210)
DEFINE_ISR(211)
DEFINE_ISR(212)
DEFINE_ISR(213)
DEFINE_ISR(214)
DEFINE_ISR(215)
DEFINE_ISR(216)
DEFINE_ISR(217)
DEFINE_ISR(218)
DEFINE_ISR(219)
DEFINE_ISR(220)
DEFINE_ISR(221)
DEFINE_ISR(222)
DEFINE_ISR(223)
DEFINE_ISR(224)
DEFINE_ISR(225)
DEFINE_ISR(226)
DEFINE_ISR(227)
DEFINE_ISR(228)
DEFINE_ISR(229)
DEFINE_ISR(230)
DEFINE_ISR(231)
DEFINE_ISR(232)
DEFINE_ISR(233)
DEFINE_ISR(234)
DEFINE_ISR(235)
DEFINE_ISR(236)
DEFINE_ISR(237)
DEFINE_ISR(238)
DEFINE_ISR(239)
DEFINE_ISR(240)
DEFINE_ISR(241)
DEFINE_ISR(242)
DEFINE_ISR(243)
DEFINE_ISR(244)
DEFINE_ISR(245)
DEFINE_ISR(246)
DEFINE_ISR(247)
DEFINE_ISR(248)
DEFINE_ISR(249)
DEFINE_ISR(250)
DEFINE_ISR(251)
DEFINE_ISR(252)
DEFINE_ISR(253)
DEFINE_ISR(254)
DEFINE_ISR(255)

void isr_init_idt_entries(void)
{
    idt_create_entry(0, (u32)isr_int0, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(1, (u32)isr_int1, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(2, (u32)isr_int2, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(3, (u32)isr_int3, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(4, (u32)isr_int4, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(5, (u32)isr_int5, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(6, (u32)isr_int6, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(7, (u32)isr_int7, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(8, (u32)isr_int8, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(9, (u32)isr_int9, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(10,(u32) isr_int10, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(11,(u32) isr_int11, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(12,(u32) isr_int12, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(13,(u32) isr_int13, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(14,(u32) isr_int14, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(15,(u32) isr_int15, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(16,(u32) isr_int16, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(17,(u32) isr_int17, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(18,(u32) isr_int18, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(19,(u32) isr_int19, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(20,(u32) isr_int20, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(21,(u32) isr_int21, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(22,(u32) isr_int22, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(23,(u32) isr_int23, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(24,(u32) isr_int24, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(25,(u32) isr_int25, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(26,(u32) isr_int26, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(27,(u32) isr_int27, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(28,(u32) isr_int28, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(29,(u32) isr_int29, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(30,(u32) isr_int30, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(31,(u32) isr_int31, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(32,(u32) isr_int32, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(33,(u32) isr_int33, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(34,(u32) isr_int34, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(35,(u32) isr_int35, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(36,(u32) isr_int36, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(37,(u32) isr_int37, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(38,(u32) isr_int38, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(39,(u32) isr_int39, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(40,(u32) isr_int40, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(41,(u32) isr_int41, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(42,(u32) isr_int42, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(43,(u32) isr_int43, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(44,(u32) isr_int44, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(45,(u32) isr_int45, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(46,(u32) isr_int46, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(47,(u32) isr_int47, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(48,(u32) isr_int48, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(49,(u32) isr_int49, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(50,(u32) isr_int50, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(51,(u32) isr_int51, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(52,(u32) isr_int52, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(53,(u32) isr_int53, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(54,(u32) isr_int54, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(55,(u32) isr_int55, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(56,(u32) isr_int56, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(57,(u32) isr_int57, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(58,(u32) isr_int58, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(59,(u32) isr_int59, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(60,(u32) isr_int60, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(61,(u32) isr_int61, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(62,(u32) isr_int62, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(63,(u32) isr_int63, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(64,(u32) isr_int64, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(65,(u32) isr_int65, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(66,(u32) isr_int66, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(67,(u32) isr_int67, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(68,(u32) isr_int68, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(69,(u32) isr_int69, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(70,(u32) isr_int70, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(71,(u32) isr_int71, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(72,(u32) isr_int72, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(73,(u32) isr_int73, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(74,(u32) isr_int74, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(75,(u32) isr_int75, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(76,(u32) isr_int76, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(77,(u32) isr_int77, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(78,(u32) isr_int78, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(79,(u32) isr_int79, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(80,(u32) isr_int80, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(81,(u32) isr_int81, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(82,(u32) isr_int82, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(83,(u32) isr_int83, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(84,(u32) isr_int84, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(85,(u32) isr_int85, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(86,(u32) isr_int86, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(87,(u32) isr_int87, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(88,(u32) isr_int88, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(89,(u32) isr_int89, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(90,(u32) isr_int90, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(91,(u32) isr_int91, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(92,(u32) isr_int92, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(93,(u32) isr_int93, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(94,(u32) isr_int94, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(95,(u32) isr_int95, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(96,(u32) isr_int96, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(97,(u32) isr_int97, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(98,(u32) isr_int98, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(99,(u32) isr_int99, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(100, (u32)isr_int100, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(101, (u32)isr_int101, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(102, (u32)isr_int102, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(103, (u32)isr_int103, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(104, (u32)isr_int104, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(105, (u32)isr_int105, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(106, (u32)isr_int106, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(107, (u32)isr_int107, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(108, (u32)isr_int108, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(109, (u32)isr_int109, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(110, (u32)isr_int110, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(111, (u32)isr_int111, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(112, (u32)isr_int112, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(113, (u32)isr_int113, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(114, (u32)isr_int114, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(115, (u32)isr_int115, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(116, (u32)isr_int116, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(117, (u32)isr_int117, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(118, (u32)isr_int118, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(119, (u32)isr_int119, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(120, (u32)isr_int120, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(121, (u32)isr_int121, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(122, (u32)isr_int122, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(123, (u32)isr_int123, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(124, (u32)isr_int124, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(125, (u32)isr_int125, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(126, (u32)isr_int126, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(127, (u32)isr_int127, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(128, (u32)isr_int128, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(129, (u32)isr_int129, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(130, (u32)isr_int130, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(131, (u32)isr_int131, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(132, (u32)isr_int132, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(133, (u32)isr_int133, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(134, (u32)isr_int134, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(135, (u32)isr_int135, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(136, (u32)isr_int136, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(137, (u32)isr_int137, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(138, (u32)isr_int138, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(139, (u32)isr_int139, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(140, (u32)isr_int140, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(141, (u32)isr_int141, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(142, (u32)isr_int142, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(143, (u32)isr_int143, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(144, (u32)isr_int144, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(145, (u32)isr_int145, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(146, (u32)isr_int146, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(147, (u32)isr_int147, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(148, (u32)isr_int148, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(149, (u32)isr_int149, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(150, (u32)isr_int150, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(151, (u32)isr_int151, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(152, (u32)isr_int152, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(153, (u32)isr_int153, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(154, (u32)isr_int154, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(155, (u32)isr_int155, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(156, (u32)isr_int156, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(157, (u32)isr_int157, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(158, (u32)isr_int158, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(159, (u32)isr_int159, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(160, (u32)isr_int160, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(161, (u32)isr_int161, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(162, (u32)isr_int162, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(163, (u32)isr_int163, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(164, (u32)isr_int164, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(165, (u32)isr_int165, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(166, (u32)isr_int166, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(167, (u32)isr_int167, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(168, (u32)isr_int168, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(169, (u32)isr_int169, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(170, (u32)isr_int170, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(171, (u32)isr_int171, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(172, (u32)isr_int172, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(173, (u32)isr_int173, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(174, (u32)isr_int174, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(175, (u32)isr_int175, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(176, (u32)isr_int176, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(177, (u32)isr_int177, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(178, (u32)isr_int178, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(179, (u32)isr_int179, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(180, (u32)isr_int180, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(181, (u32)isr_int181, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(182, (u32)isr_int182, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(183, (u32)isr_int183, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(184, (u32)isr_int184, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(185, (u32)isr_int185, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(186, (u32)isr_int186, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(187, (u32)isr_int187, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(188, (u32)isr_int188, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(189, (u32)isr_int189, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(190, (u32)isr_int190, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(191, (u32)isr_int191, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(192, (u32)isr_int192, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(193, (u32)isr_int193, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(194, (u32)isr_int194, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(195, (u32)isr_int195, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(196, (u32)isr_int196, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(197, (u32)isr_int197, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(198, (u32)isr_int198, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(199, (u32)isr_int199, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(200, (u32)isr_int200, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(201, (u32)isr_int201, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(202, (u32)isr_int202, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(203, (u32)isr_int203, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(204, (u32)isr_int204, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(205, (u32)isr_int205, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(206, (u32)isr_int206, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(207, (u32)isr_int207, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(208, (u32)isr_int208, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(209, (u32)isr_int209, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(210, (u32)isr_int210, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(211, (u32)isr_int211, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(212, (u32)isr_int212, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(213, (u32)isr_int213, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(214, (u32)isr_int214, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(215, (u32)isr_int215, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(216, (u32)isr_int216, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(217, (u32)isr_int217, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(218, (u32)isr_int218, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(219, (u32)isr_int219, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(220, (u32)isr_int220, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(221, (u32)isr_int221, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(222, (u32)isr_int222, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(223, (u32)isr_int223, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(224, (u32)isr_int224, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(225, (u32)isr_int225, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(226, (u32)isr_int226, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(227, (u32)isr_int227, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(228, (u32)isr_int228, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(229, (u32)isr_int229, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(230, (u32)isr_int230, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(231, (u32)isr_int231, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(232, (u32)isr_int232, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(233, (u32)isr_int233, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(234, (u32)isr_int234, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(235, (u32)isr_int235, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(236, (u32)isr_int236, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(237, (u32)isr_int237, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(238, (u32)isr_int238, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(239, (u32)isr_int239, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(240, (u32)isr_int240, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(241, (u32)isr_int241, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(242, (u32)isr_int242, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(243, (u32)isr_int243, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(244, (u32)isr_int244, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(245, (u32)isr_int245, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(246, (u32)isr_int246, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(247, (u32)isr_int247, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(248, (u32)isr_int248, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(249, (u32)isr_int249, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(250, (u32)isr_int250, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(251, (u32)isr_int251, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(252, (u32)isr_int252, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(253, (u32)isr_int253, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(254, (u32)isr_int254, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
    idt_create_entry(255, (u32)isr_int255, KERNEL_CODE_SEG, IDT_ATTR_GATE_INT);
}