        INTEGER ASF(13)
        REAL X(10), Y(10), CPX, CPY, TX(4), TY(4)
        INTEGER I, J, K      
        INTEGER COLIA(8, 10) 
        INTEGER JERR, TNR

        DATA ASF /13*1/

C* initialize color index array
        K = 0
        DO 200 J = 1, 10
          DO 300 I = 1, 8
             COLIA(I, J) = K
             K = K+1
 300      CONTINUE
 200    CONTINUE

        CALL GOPKS(5, -1)
        CALL GOPWK(1, 0, 0)
        CALL GACWK(1)

        CALL GSASF(ASF)
        CALL GQCNTN(JERR, TNR)
        CALL GSELNT(0)
        CALL GSTXFP(12, 2)
C* text
        CALL GSCHH(0.02)
        CALL GSTXFP(-1, 2)
        CALL GTX(.6, .7, 'Font-1')
        CALL GSTXFP(-2, 2)
        CALL GTX(.6, .65, 'Font-2')
        CALL GSTXFP(-3, 2)
        CALL GTX(.6, .6, 'Font-3')
        CALL GSTXFP(-4, 2)
        CALL GTX(.6, .55, 'Font-4')
        CALL GSTXFP(-5, 2)
        CALL GTX(.6, .5, 'Font-5')
        CALL GSTXFP(-6, 2)
        CALL GTX(.6, .45, 'Font-6')
        CALL GSTXFP(-7, 2)
        CALL GTX(.6, .4, 'Font-7')
        CALL GSTXFP(-8, 2)
        CALL GTX(.6, .35, 'Font-8')
        CALL GSTXFP(-9, 2)
        CALL GTX(.6, .3, 'Font-9')
        CALL GSTXFP(-10, 2)
        CALL GTX(.6, .25, 'Font-10')
        CALL GSTXFP(-11, 2)
        CALL GTX(.6, .2, 'Font-11')
        CALL GSTXFP(-12, 2)
        CALL GTX(.8, .7, 'Font-12')
        CALL GSTXFP(-13, 2)
        CALL GTX(.8, .65, 'Font-13')
        CALL GSTXFP(-14, 2)
        CALL GTX(.8, .6, 'Font-14')
        CALL GSTXFP(-15, 2)
        CALL GTX(.8, .55, 'Font-15')
        CALL GSTXFP(-16, 2)
        CALL GTX(.8, .5, 'Font-16')
        CALL GSTXFP(-17, 2)
        CALL GTX(.8, .45, 'Font-17')
        CALL GSTXFP(-18, 2)
        CALL GTX(.8, .4, 'Font-18')
        CALL GSTXFP(-19, 2)
        CALL GTX(.8, .35, 'Font-19')
        CALL GSTXFP(-20, 2)
        CALL GTX(.8, .3, 'Font-20')
        CALL GSTXFP(-21, 2)
        CALL GTX(.8, .25, 'Font-21')
        CALL GSTXFP(-22, 2)
        CALL GTX(.8, .2, 'Font-22')
        CALL GSTXFP(-23, 2)
        CALL GTX(.6, .15, 'Font-23')
        CALL GSTXFP(-24, 2)
        CALL GTX(.8, .15, 'Font-24')

C* colors
        CALL GSTXFP(-5, 2)
        CALL GSTXCI(0)
        CALL GTX(.45, .35, 'White')
        CALL GSTXCI(1)
        CALL GTX(.45, .32, 'Black')
        CALL GSTXCI(2)
        CALL GTX(.45, .29, 'Red')
        CALL GSTXCI(3)
        CALL GTX(.45, .26, 'Green')
        CALL GSTXCI(4)
        CALL GTX(.45, .23, 'Blue')
        CALL GSTXCI(5)
        CALL GTX(.45, .20, 'Cyan')
        CALL GSTXCI(6)
        CALL GTX(.45, .17, 'Yellow')
        CALL GSTXCI(7)
        CALL GTX(.45, .14, 'Magenta')

C* linetypes
        X(1) = 0.10
        X(2) = 0.26
        Y(1) = 0.95
        Y(2) = Y(1)

        DO 100, J = -8, 4
          IF (J .EQ. 0) GOTO 100
          CALL GSLN(J)
          Y(1) = Y(1) - 0.05
          Y(2) = Y(1)
          CALL GPL(2, X, Y)
 100    CONTINUE 

C* markertypes
        CALL GSMKSC(3.5)

        X(1) = 0.25
        Y(1) = 0.95
        DO 150, J = -32, -21
          CALL GSMK(J)
          X(1) = X(1)+0.06
          CALL GPM(1, X, Y)
 150    CONTINUE 

        X(1) = 0.25
        Y(1) = 0.875

        DO 160, J = -20, -9
          CALL GSMK(J)
          X(1) = X(1)+0.06
          CALL GPM(1, X, Y)
 160    CONTINUE 

        X(1) = 0.25
        Y(1) = 0.8

        DO 170, J = -8, 4
          IF (J .EQ. 0) GOTO 170
          CALL GSMK(J)
          X(1) = X(1)+0.06
          CALL GPM(1, X, Y)
 170    CONTINUE 

C* fill areas
        X(1) = 0.02
        X(2) = 0.12
        X(3) = 0.12
        X(4) = 0.02
        X(5) = X(1)
        Y(1) = 0.02
        Y(2) = 0.02
        Y(3) = 0.12
        Y(4) = 0.12
        Y(5) = Y(1)
        
        CALL GSFASI(4)
        DO 20, J = 0, 3
          DO 10, I = 1, 5
            X(I)=X(I)+0.1
            Y(I)=Y(I)+0.1
  10      CONTINUE
          CALL GSFAIS(J)
          CALL GFA(5, X, Y)
  20    CONTINUE

C* patterns
        X(1) = 0.02
        X(2) = 0.07
        X(3) = 0.07
        X(4) = 0.02
        X(5) = X(1)
        Y(1) = 0.2
        Y(2) = 0.2
        Y(3) = 0.25
        Y(4) = 0.25
        Y(5) = Y(1)
        CALL GSFAIS(2)
        DO 40, J = 4, 15
          DO 30, I = 1, 5
            Y(I)=Y(I)+0.06
  30      CONTINUE
          CALL GSFASI(J-3)
          CALL GFA(5, X, Y)
  40    CONTINUE

C* cell array
        CALL GCA(0.332, 0.75, 0.532, 0.55, 8, 10, 1, 1, 8, 10, COLIA)   
                
C* more text
        CALL GSCHH(0.08)
        CALL GSTXAL(2, 3)
        CALL GSTXCI(1)
        CALL GSTXFP(12, 2)
        CALL GTX(.5, .05, 'Hello World')

        CALL GQTXX(1, .5, .05, 'Hello World', JERR, CPX, CPY, TX, TY)
        IF (JERR .EQ. 0) THEN
          CALL GSFAIS(0)
          CALL GFA(4, TX, TY)
        END IF

        CALL GSCHH(0.024)
        CALL GSTXAL(1, 1)
        CALL GSTXFP(3, 0)
        CALL GSCHUP(-1., 0.)
        CALL GTX(.05, .15, 'up')
        CALL GSCHUP(0., -1.)
        CALL GTX(.05, .15, 'left')
        CALL GSCHUP(1., 0.)
        CALL GTX(.05, .15, 'down')
        CALL GSCHUP(0., 1.)
        CALL GTX(.05, .15, 'right')

        CALL GUWK(1, 0)
        READ(*, *)

        CALL GECLKS
        
        END
