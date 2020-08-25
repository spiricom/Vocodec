/*
 * tunings.c
 *
 *  Created on: Dec 18, 2019
 *      Author: josnyder
 */

#ifndef __cplusplus
#include "main.h"
#endif

#include "tunings.h"

#ifdef __cplusplus
namespace vocodec
{
    extern "C"
    {
#endif

        float centsDeviation[12] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
        uint32_t currentTuning = 0;
        uint8_t keyCenter = 0;
        const char tuningNames[NUM_TUNINGS][13]= {
            {"12-TET      "},
            {"JustOvertone"},
            {"Kirnberger3"},
            {"Meantone1/4"},
            {"JI_12"},
            {"Agricola_P"},
            {"Kora1"},
            {"Kora2"},
            {"Kora3"},
            {"Kora4"},
            {"Johnston"},
            {"Mambuti"},
            {"Marimba3"},
            {"Mbira1"},
            {"Mbira2"},
            {"Mbira3"},
            {"Mbira4"},
            {"Mbira5"},
            {"LMY_piano"},
            {"Xylo2"},
            {"Xylo4"},
            {"LMY_guitar"},
            {"Abell1"},
            {"Aeolic"},
            {"Pelog"},
            {"Slendro"},
            {"Angklung"},
            {"Huzam"},
            {"Arch_chrom"},
            {"Archytas12"},
            {"Arch_ptol"},
            {"Arch_sept"},
            {"Ariel3"},
            {"Augtetj"},
            {"Awraamoff"},
            {"Bagpipe1"},
            {"Bagpipe2"},
            {"Bagpipe3"},
            {"Balafon"},
            {"Balafon2"},
            {"Balafon3"},
            {"Balafon4"},
            {"Bellingwolde"},
            {"Johnston_6"},
            {"Keenan_t9"},
            {"Lara"},
            {"Ligon"},
            {"Lydian_di2"},
            {"Lydian_di2I"},
            {"Lydian_enh2"},
            {"Lydian_enhI"},
            {"Malcolms"},
            {"Met24-chrys"},
            {"Metals"},
            {"Ptolemy_imix"},
            {"Riley_albion"},
            {"Riley_rosary"},
            {"Couperin"},
            {"Dowland"},
            {"BluesMarv"},
            {"Carlos_Harm"},
            {"Partch_Grm"},
            {"Partch_Greek"},
        };

        //12_TET                            12 tone equal temperament
        //jeff Just                        12 custom simple overtone just scale
        //kirnberger.scl                 12  Kirnberger's well-temperament, also called Kirnberger III, letter to Forkel 1779
        //meanquar.scl                   12  1/4-comma meantone scale. Pietro Aaron's temp. (1523). 6/5 beats twice 3/2
        //ji_12.scl                      12  Basic JI with 7-limit tritone. Robert Rich: Geometry
        //agricola_p.scl                 12  Agricola's Pythagorean-type Monochord, Musica instrumentalis deudsch (1545)
        //kora1.scl                       7  Kora tuning Tomora Ba, also called Silaba, 1/1=F, R. King
        //kora2.scl                       7  Kora tuning Tomora Mesengo, also called Tomora, 1/1=F, R. King
        //kora3.scl                       7  Kora tuning Hardino, 1/1=F, R.King
        //kora4.scl                       7  Kora tuning Sauta (Sawta), 1/1=F, R. King
        //johnston.scl                   12  Ben Johnston's combined otonal-utonal scale
        //mambuti.scl                     8  African Mambuti Flutes (aerophone; vertical wooden; one note each)
        //marimba3.scl                   10  Marimba from the Yakoma tribe, Zaire. 1/1=185.5 Hz
        //mbira_banda.scl                 7  Mubayiwa Bandambira's tuning of keys R2-R9 from Berliner: The soul of mbira.
        //mbira_chilimba.scl              7  Mbira chilimba from Bemba. 1/1=228 Hz, Tracey TR-182 B-7
        //mbira_chisanzhi2.scl            7  Mbira chisanzhi from Lunda. 1/1=212 Hz, Tracey TR-179 B-5,6
        //mbira_kunaka.scl                7  John Kunaka's mbira tuning of keys R2-R9
        //mbira_neikembe.scl              7  Mbira neikembe from Medje. 1/1=320 Hz, Tracey TR-120 B-1,2
        //young-lm_piano.scl             12  LaMonte Young's Well-Tuned Piano
        //xylophone2.scl                 10  African Yaswa xylophones (idiophone; calbash resonators with membrane)
        //xylophone4.scl                 10  African Bapare xylophone (idiophone; loose log)
        //young-lm_guitar.scl            12  LaMonte Young, tuning of For Guitar '58. 1/1 March '92, inv.of Mersenne lute 1
        //abell1.scl                     12  Ross Abell's French Baroque Meantone 1, a'=520 Hz
        //aeolic.scl                      7  Ancient Greek Aeolic, also tritriadic scale of the 54:64:81 triad
        //alves_pelog.scl                 7  Bill Alves JI Pelog, 1/1 vol.9 no.4, 1997. 1/1=293.33 Hz
        //alves_slendro.scl               5  Bill Alves, slendro for Gender Barung, 1/1 vol.9 no.4, 1997. 1/1=282.86 Hz
        //angklung.scl                    8  Scale of an anklung set from Tasikmalaya. 1/1=174 Hz
        //arabic_huzam_on_e.scl          12  Arabic Huzam with perde segah on E by Dr. Ozan Yarman.
        //arch_chrom.scl                  7  Archytas' Chromatic
        //archytas12sync.scl             12  Archytas[12] (64/63) hobbit, sync beating
        //arch_ptol.scl                  12  Archytas/Ptolemy Hybrid 1
        //arch_sept.scl                  12  Archytas Septimal
        //ariel3.scl                     12  Ariel's 12-tone JI scale
        //augtetj.scl                     6  9/8 C.I. comprised of 11:10:9:8 subharmonic series on 1 and 8:9:10:11 on 16/11
        //awraamoff.scl                  12  Awraamoff Septimal Just (1920)
        //bagpipe1.scl                   12  Bulgarian bagpipe tuning
        //bagpipe2.scl                    9  Highland Bagpipe, from Acustica4: 231 (1954) J.M.A Lenihan and S. McNeill
        //bagpipe3.scl                    9  Highland Bagpipe, Allan Chatto, 1991. From Australian Pipe Band College
        //balafon.scl                     7  Observed balafon tuning from Patna, Helmholtz/Ellis p. 518, nr.81
        //balafon2.scl                    7  Observed balafon tuning from West-Africa, Helmholtz/Ellis p. 518, nr.86
        //balafon3.scl                    7  Pitt-River's balafon tuning from West-Africa, Helmholtz/Ellis p. 518, nr.87
        //balafon4.scl                    7  Mandinka balafon scale from Gambia
        //bellingwolde.scl               12  Current 1/6-P. comma mod.mean of Freytag organ in Bellingwolde. Ortgies,2002
        //johnston_6-qt_row.scl          12  11-limit 'prime row' from Ben Johnston's "6th Quartet"
        //keenan_t9.scl                  12  Dave Keenan strange 9-limit temperament TL 19-11-98
        //lara.scl                       12  Sundanese 'multi-laras' gamelan Ki Barong tuning, Weintraub, TL 15-2-99 1/1=497
        //ligon.scl                      12  Jacky Ligon, strictly proper all prime scale, TL 08-09-2000
        //lydian_diat2.scl                8  Schlesinger's Lydian Harmonia, a subharmonic series through 13 from 26
        //lydian_diat2inv.scl             8  Inverted Schlesinger's Lydian Harmonia, a harmonic series from 13 from 26
        //lydian_enh2.scl                 7  Schlesinger's Lydian Harmonia in the enharmonic genus
        //lydian_enhinv.scl               7  A harmonic form of Schlesinger's Enharmonic Lydian inverted
        //malcolms.scl                   12  Symmetrical version of Malcolm's Monochord and Riley's Albion scale.
        //met24-chrys_diat-4th_pl.scl     7  Near Chrysanthos 4th Byzantine Liturgical mode, JI
        //metals.scl                      9  Gold, silver, titanium - strong metastable intervals between 1 and 2.
        //ptolemy_imix.scl               11  Ptolemy Intense Diatonic mixed with its inverse
        //riley_albion.scl               12  Terry Riley's Harp of New Albion scale, inverse Malcolm's Monochord, 1/1 on C#
        //riley_rosary.scl               12  Terry Riley, tuning for Cactus Rosary (1993)
        // couperin.scl                   12  Couperin modified meantone
        //dowland_12.scl                 12  subset of Dowland's lute tuning, lowest octave
        //bluesmarvwoo.scl               12  Marvel woo version of Graham Breed's Blues scale
        //carlos_harm.scl            12  Carlos Harmonic & Ben Johnston's scale of 'Blues' from Suite f.micr.piano (1977) & David Beardsley's scale of 'Science Friction'
        //partch-grm.scl                  9  Partch Greek scales from "Two Studies on Ancient Greek Scales" mixed
        //partch-greek.scl               12  Partch Greek scales from "Two Studies on Ancient Greek Scales" on black/white

        //encoded as deviations in semitones from scale position - always a full 12 note scale, repeating notes if there are fewer than 12 in scale.
        const float tuningPresets[NUM_TUNINGS][12] = {
            {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
            {0.000000f, 0.110000f, 0.030000f, 0.150000f, -0.140000f, -0.020000f, -0.490000f, 0.010000f, 0.130000f, -0.160000f, -0.320000f, -0.120000f},
            {0.000000f, -0.097750f, -0.068431f, -0.058650f, -0.136863f, -0.019550f, -0.097763f, -0.034216f, -0.078200f, -0.102647f, -0.039100f, -0.117313f},
            {0.000000f, -0.239510f, -0.068431f, 0.102647f, -0.136863f, 0.034216f, -0.205294f, -0.034216f, -0.273726f, -0.102647f, 0.068431f, -0.171079f},
            {0.000000f, 0.117313f, 0.039100f, 0.156413f, -0.136863f, -0.019550f, -0.174878f, 0.019550f, 0.136863f, -0.156413f, 0.175963f, -0.117313f},
            {0.000000f, -0.080000f, 0.030000f, -0.040000f, 0.070000f, -0.020000f, -0.100000f, 0.010000f, -0.060000f, 0.050000f, -0.040000f, 0.090000f},
            {0.000000f, -1.000000f, 0.000000f, -1.000000f, -0.150000f, 0.000000f, 1.000000f, 0.000000f, 1.000000f, 0.000000f, 0.850000f, -0.150000f},
            {0.000000f, -1.000000f, 0.300000f, -0.700000f, -0.750000f, 0.000000f, 1.000000f, 0.000000f, 1.300000f, 0.300000f, 0.250000f, -0.750000f},
            {0.000000f, -1.000000f, -0.150000f, -1.150000f, 0.050000f, 0.000000f, 1.000000f, 0.000000f, 0.850000f, -0.150000f, 1.050000f, 0.050000f},
            {0.000000f, -1.000000f, -0.150000f, -1.150000f, 0.050000f, 1.050000f, 1.000000f, 0.000000f, 0.850000f, -0.150000f, 1.050000f, 0.050000f},
            {0.000000f, -0.293276f, -0.175963f, -0.254176f, -0.136863f, 0.057565f, -0.312826f, 0.337217f, -0.273726f, -0.156413f, -0.767355f, -0.117313f},
            {0.000000f, -1.000000f, 0.040000f, -0.960000f, 0.110000f, 2.100000f, 4.000000f, 3.000000f, 4.060000f, 5.090000f, 4.090000f, 8.180000f},
            {0.000000f, -1.000000f, 0.180000f, 1.950000f, 4.200000f, 5.380000f, 5.850000f, 7.470000f, 8.950000f, 11.200000f, 12.380000f, 11.380000f},
            {0.000000f, -1.000000f, -0.150000f, -1.150000f, -0.110000f, 0.930000f, 1.560000f, 0.560000f, 1.140000f, 0.140000f, 0.510000f, -0.490000f},
            {0.000000f, -1.000000f, -0.270000f, -1.270000f, -0.450000f, 0.200000f, 0.920000f, -0.080000f, 0.660000f, -0.340000f, 0.740000f, -0.260000f},
            {0.000000f, -1.000000f, -0.439274f, -1.439274f, -0.071656f, -0.183649f, 0.240085f, -0.759915f, 0.380132f, -0.619868f, 0.103222f, -0.896778f},
            {0.000000f, -1.000000f, -0.040000f, -1.040000f, -0.230000f, 0.060000f, 0.760000f, -0.240000f, 0.770000f, -0.230000f, 0.500000f, -0.500000f},
            {0.000000f, -1.000000f, 0.040000f, -0.960000f, -0.310000f, 0.200000f, 1.020000f, 0.020000f, 0.670000f, -0.330000f, 0.660000f, -0.340000f},
            {0.000000f, 0.760000f, 0.030000f, -0.610000f, 0.700000f, -0.570000f, 0.740000f, 0.010000f, -0.630000f, 0.680000f, -0.590000f, 0.720000f},
            {0.000000f, -1.000000f, 0.090000f, 1.160000f, 2.860000f, 4.260000f, 5.440000f, 5.130000f, 5.770000f, 6.300000f, 8.260000f, 7.260000f},
            {0.000000f, -1.000000f, -0.720000f, 0.170000f, 1.020000f, 1.990000f, 2.880000f, 4.410000f, 5.450000f, 5.310000f, 6.040000f, 5.040000f},
            {0.000000f, 0.110000f, -0.180000f, 0.150000f, -0.140000f, -0.020000f, -0.100000f, 0.010000f, 0.130000f, -0.160000f, 0.170000f, -0.120000f},
            {0.000000f, -0.220000f, -0.060000f, -0.290000f, -0.130000f, -0.360000f, -0.190000f, -0.030000f, -0.250000f, -0.090000f, -0.320000f, -0.160000f},
            {0.000000f, -1.000000f, 0.030000f, -0.970000f, -1.060000f, -0.020000f, 1.010000f, 0.010000f, -0.080000f, -1.080000f, -0.040000f, -1.040000f},
            {0.000000f, -1.000000f, 0.310000f, -0.690000f, -0.850000f, -0.300000f, 1.010000f, 0.010000f, 0.130000f, -0.870000f, -0.320000f, -1.320000f},
            {0.000000f, -1.000000f, 0.310000f, -0.690000f, -1.690000f, -0.020000f, -1.020000f, 0.640000f, -0.360000f, 0.960000f, -0.040000f, -1.040000f},
            {0.000000f, -1.000000f, 0.060000f, -0.940000f, -0.180000f, 1.100000f, 2.230000f, 1.230000f, 4.340000f, 5.060000f, 4.060000f, 5.330000f},
            {0.000000f, 1.350000f, -0.010000f, 0.010000f, -0.490000f, 0.010000f, -0.320000f, 0.010000f, 0.490000f, 0.010000f, 0.000000f, 0.020000f},
            {0.000000f, -1.000000f, -1.380000f, -2.380000f, -1.970000f, -0.020000f, 1.010000f, 0.010000f, -0.360000f, -1.360000f, -0.950000f, -1.950000f},
            {0.000000f, -0.040000f, 0.220000f, 0.190000f, -0.080000f, -0.120000f, 0.150000f, 0.110000f, 0.070000f, 0.340000f, -0.230000f, 0.030000f},
            {0.000000f, -0.380000f, -0.890000f, -1.180000f, -1.060000f, -0.020000f, -0.390000f, 0.010000f, -0.360000f, -0.870000f, -1.160000f, -1.040000f},
            {0.000000f, -0.380000f, -0.890000f, -0.970000f, -1.060000f, -0.020000f, -0.390000f, 0.010000f, -0.360000f, -0.870000f, -0.950000f, -1.040000f},
            {0.000000f, 0.110000f, -0.180000f, -0.060000f, -0.360000f, -0.020000f, -0.320000f, 0.010000f, 0.130000f, -0.160000f, -0.040000f, -0.340000f},
            {0.000000f, -1.000000f, -0.350000f, -1.350000f, -0.530000f, -1.530000f, -0.490000f, -1.490000f, -1.520000f, -2.520000f, -1.480000f, -2.480000f},
            {0.000000f, 1.030000f, 0.310000f, 0.150000f, -0.140000f, -0.300000f, -1.020000f, 0.010000f, 0.130000f, 0.330000f, -0.320000f, -0.120000f},
            {0.000000f, -3.030000f, -2.000000f, -3.000000f, -1.970000f, -1.140000f, -2.140000f, -1.810000f, -0.990000f, -1.990000f, -1.160000f, -0.830000f},
            {0.000000f, -2.820000f, -2.000000f, -3.000000f, -1.970000f, -1.140000f, -2.140000f, -2.020000f, -0.990000f, -1.990000f, -1.160000f, -0.830000f},
            {0.000000f, -3.310000f, -2.000000f, -3.000000f, -1.970000f, -1.140000f, -2.140000f, -2.020000f, -0.990000f, -1.990000f, -1.160000f, -1.320000f},
            {0.000000f, -1.000000f, -0.130000f, -1.130000f, -0.440000f, 0.260000f, 0.720000f, -0.280000f, 0.560000f, -0.440000f, -0.150000f, -1.150000f},
            {0.000000f, -1.000000f, -0.480000f, -1.480000f, -1.130000f, 0.330000f, 1.240000f, 0.240000f, 0.900000f, -0.100000f, 0.390000f, -0.610000f},
            {0.000000f, -1.000000f, -0.050000f, -1.050000f, -1.110000f, 0.130000f, 0.860000f, -0.140000f, -0.040000f, -1.040000f, 0.080000f, -0.920000f},
            {0.000000f, -1.000000f, -0.490000f, -1.490000f, -0.550000f, 0.260000f, 0.600000f, -0.400000f, 0.610000f, -0.390000f, 0.250000f, -0.750000f},
            {0.000000f, -0.100000f, -0.040000f, 0.010000f, -0.080000f, 0.010000f, -0.120000f, -0.020000f, -0.040000f, -0.060000f, 0.000000f, -0.100000f},
            {0.000000f, -0.078213f, 0.039100f, 0.233528f, -0.136863f, 0.513179f, -0.097763f, 0.019550f, -0.591392f, 0.058650f, -0.311741f, -0.117313f},
            {0.000000f, 0.060000f, 0.120000f, -0.240000f, -0.180000f, -0.120000f, 0.000000f, 0.060000f, 0.120000f, -0.240000f, -0.180000f, -0.120000f},
            {0.000000f, 0.570000f, 0.270000f, 0.110000f, 0.590000f, 1.330000f, 1.150000f, 1.200000f, 1.390000f, 1.960000f, 2.000000f, 2.700000f},
            {0.000000f, 0.154584f, -0.074424f, -0.107903f, 0.013028f, -0.355723f, -0.174878f, 0.019550f, -0.175080f, -0.156413f, -0.122533f, -0.282982f},
            {0.000000f, -1.000000f, -0.614273f, -1.614273f, -1.107903f, -0.457861f, -0.569854f, -1.569854f, -1.633823f, -0.594723f, -1.594723f, -0.282982f},
            {0.000000f, -1.000000f, -0.717018f, -1.717018f, -0.405277f, 0.633823f, 0.569854f, -0.430146f, -0.542139f, 0.107903f, -0.892097f, -0.385727f},
            {0.000000f, -1.000000f, -1.663827f, -2.663827f, -3.320998f, -0.457861f, 0.366177f, -0.633823f, -1.146120f, -2.146120f, -2.644277f, -3.644277f},
            {0.000000f, -1.000000f, -1.670231f, -2.670231f, -3.346627f, 0.633823f, 1.457861f, 0.457861f, -0.114653f, -1.114653f, -1.697468f, -2.697468f},
            {0.000000f, 0.117313f, 0.039100f, 0.156413f, -0.136863f, -0.019550f, 0.000000f, 0.019550f, 0.136863f, -0.156413f, -0.039100f, -0.117313f},
            {0.000000f, -1.000000f, 0.074219f, -0.925781f, -0.425781f, -0.031250f, 1.042969f, 0.042969f, 1.117188f, 0.117188f, 0.617188f, -0.382812f},
            {0.000000f, 2.390000f, 2.220000f, 1.220000f, 1.600000f, 1.070000f, 0.070000f, 0.920000f, 0.330000f, -0.670000f, -0.570000f, -0.980000f},
            {0.000000f, -1.000000f, -0.882687f, -0.960900f, -0.843587f, -1.136863f, -1.019550f, 0.019550f, 0.136863f, -0.156413f, -0.039100f, -0.117313f},
            {0.000000f, 0.117313f, 0.039100f, 0.156413f, -0.136863f, -0.019550f, 0.097763f, 0.019550f, 0.136863f, -0.156413f, -0.039100f, -0.117313f},
            {0.000000f, -0.643032f, 0.039100f, -0.331291f, -0.136863f, -0.292191f, -0.486821f, 0.019550f, -0.623482f, -0.594723f, -0.311741f, -0.117313f},
            {0.000000f, -0.239510f, -0.068431f, -0.102640f, -0.136863f, 0.034216f, -0.205294f, -0.034216f, -0.273726f, -0.102647f, -0.034212f, -0.171079f},
            {0.000000f, 0.082374f, 0.039100f, -0.155530f, -0.120461f, -0.019550f, -0.030004f, 0.019550f, 0.101924f, 0.058650f, -0.135980f, -0.049554f},
            {0.000000f, 0.338826f, -0.169548f, 0.837426f, 0.505575f, -0.000271f, 0.512550f, -0.169820f, 0.345529f, -0.162845f, -0.494696f, 0.512278f},
            {0.000000f, 0.049554f, 0.039100f, -0.024870f, -0.136863f, -0.292191f, -0.486821f, 0.019550f, 0.405277f, 0.058650f, -0.311741f, -0.117313f},
            {0.000000f, -0.370391f, -0.882687f, -1.882687f, -1.960900f, -1.843587f, -2.843587f, -2.019550f, -0.980450f, -1.980450f, -2.350841f, -2.863137f},
            {0.000000f, -0.370391f, 0.039100f, -1.882687f, 0.980450f, -1.843587f, 1.019550f, 0.019550f, -0.350841f, -0.863137f, -1.863137f, 1.000000f},
        };

#ifdef __cplusplus
    }
} // extern "C"
#endif





