      subroutine transit(compo,dt0)

      include "calcium.hf"
      dimension tn(7),t(7),rf(7)
      dimension tparoi(7),rparoi(7)
      dimension maille(2),tflu(2,7)
      double precision dt0
      integer compo

      dt=4.5*dt0

      r=1.
      ro=1.
      rext=0.5
      te=10.
      deb=10.

       maille(1)=2
       maille(2)=7

      ti=0.

      do i=1,7
      t(i)=100.
      tn(i)=100.
      rf(i)=0.5
      enddo
c
c  initialisation de temperature fluide a t=0
c
        CALL cpeRE(compo,CP_TEMPS, ti, npas+1, 'tfluide', 6,
     &            t(2) , info)
            IF( info.NE. CPOK  )GO TO 9000
c
c  initialisation de la resistance thermique fluide a t=0
c
        CALL cpeRE(compo,CP_TEMPS, ti, npas+1, 'rfluide', 6,
     &            rf(2) , info)
            IF( info.NE. CPOK  )GO TO 9000
c
c  boucle temporelle jusqu'a 100.
c
      do while( ti.lT.100. )
      tf=ti+dt
c
c   lecture de la temperature de paroi entre ti et tf
c
        CALL cplRE(compo,CP_TEMPS,ti, tf, npas+1,'tparoi',  6,
     &      nval, tparoi(2), info)
            IF( info.NE. CPOK  )GO TO 9000
c
c   lecture de la resistance solide de bord entre ti et tf
c
        CALL cplRE(compo,CP_TEMPS,ti, tf, npas+1, 'rparoi', 6,
     &      nval, rparoi(2), info)
            IF( info.NE. CPOK  )GO TO 9000
c
c  calcul de la temperature a tf
c
      do i=2,7
      smb=ro/dt*tn(i)+deb*t(i-1)+tparoi(i)/(rparoi(i)+rf(i))
      t(i)=smb/(ro/dt+deb+1./(rparoi(i)+rf(i)))
      enddo

      write(6,*)'FLUID:temps=',tf,' temperature de sortie canal=',t(7)
      call flush(6)
c
c  ecriture de la temperature fluide a tf
c
        CALL cpeRE(compo,CP_TEMPS, tf, npas+1, 'tfluide', 6,
     &            t(2) , info)
            IF( info.NE.CPOK )GO TO 9000
c
c  ecriture de la resistance thermique fluide a tf
c
        CALL cpeRE(compo,CP_TEMPS, tf, npas+1, 'rfluide', 6,
     &            rf(2) , info)
            IF( info.NE.CPOK )GO TO 9000
      do i=1,7
      tflu(1,i)=t(i)
      tflu(2,i)=t(i)
      enddo

      do i=1,7 
      tn(i)=t(i)
      enddo

      ti=tf

      enddo

9000  continue
            CALL cpfin(compo,CP_ARRET,info)
      end

      subroutine perma(compo)
      include "calcium.hf"
      dimension t(7),rf(7)
      dimension tparoi(7),rparoi(7)
      integer compo
 
      deb=10.
 
      do i=1,7
      t(i)=100.
      rf(i)=0.5
      rparoi(i)=0.5
      enddo
c
c  initialisation de temperature fluide a i=0
c
      iter=0
      iconv=0
        CALL cpeRE(compo,CP_ITERATION, ti, iter  , 'tfi', 6,
     &            t(2) , info)
            IF( info.NE. CPOK  )GO TO 9000
c
c  boucle temporelle jusqu'a 100.
c
      do while( iconv .EQ. 0)
c
c   lecture de la temperature de paroi iteration iter
c
        CALL cplRE(compo,CP_ITERATION,ti, tf, iter  , 'tpi', 6,
     &      nval, tparoi(2), info)
            IF( info.NE. CPOK  )GO TO 9000
c
c  calcul de la temperature 
c
      do i=2,7
      smb=deb*t(i-1)+tparoi(i)/(rparoi(i)+rf(i))
      t(i)=smb/(deb+1./(rparoi(i)+rf(i)))
      enddo
c
c  ecriture de la temperature fluide a iter+1
c
        CALL cpeRE(compo,CP_ITERATION,ti,iter+1, 'tfi', 6,
     &            t(2) , info)
            IF( info.NE. CPOK  )GO TO 9000
 
      iter=iter+1
      write(6,*)'iter  = ',iter,' temperature de sortie canal = ',t(7)
c
c   lecture du flag de convergence iconv
c
        CALL cplEN(compo,CP_ITERATION,ti, tf, iter  , 'iconv', 1,
     &      nval, iconv  , info)
      write(6,*)"info:",info
      write(6,*)"FLUIDE:",iter,iconv
      call flush(6)

      IF( info.NE. CPOK  )GO TO 9000
 
      if(iconv.eq.1)go to 9000

      enddo
 
9000  continue
            CALL cpfin(compo,CP_ARRET,info)
      end

