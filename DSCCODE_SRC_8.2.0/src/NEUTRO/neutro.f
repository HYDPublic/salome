      subroutine transit(compo,dt0)
      include "calcium.hf"
      dimension q(24),temp(24)
      double precision dt0
      integer compo
      integer topo(20)

      dt=dt0

      ti=0.

      do j=1,6
      npt=j*4
      q(npt)=0.
      enddo

      pos=1.
      do i=1,3
      do j=1,6
      npt=i+(j-1)*4
      haut=float(j)/6.
        if(haut.gt.pos)then
        q(npt)=0.
        else
        q(npt)=100.
        endif
      enddo
      enddo

      do i=1,20
        topo(i)=i
      enddo
c
c  initialisation puissance a t=0
c
      call cpeen(compo,CP_TEMPS,ti,1,"topo",20,topo,info)
      CALL cpeRE(compo,CP_TEMPS, ti, 1, 'puissance', 24,
     &            q , info)
      IF( info.NE. CPOK  )GO TO 9000

      do while( .TRUE. )
c     do while( ti.lT.100. )

      tf=ti+dt
c
c  lecture de la temperature combustible entre ti et tf
c
      CALL cplRE(compo,CP_TEMPS, ti,tf, npas, 'temperature', 24,
     &      nval, temp , info)
      IF( info.NE. CPOK  )GO TO 9000

      do j=1,6
      npt=j*4
      q(npt)=0.
      enddo
c
c  calcul de la puissance degagee en fonction de la position
c    des barres et de la temperature
c
      do i=1,3
      do j=1,6
      npt=i+(j-1)*4
      haut=float(j)/6.
        if(haut.gt.pos)then
        q(npt)=0.
        else
        q(npt)=100.*(1.-0.0001*(temp(npt)-1000.))
        endif
      enddo
      enddo
      write(6,*)"NEUTRO:","temps=",tf
c
c  ecriture de la puissance a tf
c
      call cpeen(compo,CP_TEMPS,ti,1,"topo",20,topo,info)
      CALL cpeRE(compo,CP_TEMPS, tf, npas+1, 'puissance', 24,
     &            q , info)
            IF( info.NE. CPOK  )GO TO 9000
      ti=tf

      enddo
9000  continue
            CALL cpfin(compo,CP_ARRET, info)
      end

      subroutine perma(compo)
      include "calcium.hf"
      dimension q(24),temp(24)
      integer compo
 
      do j=1,6
      npt=j*4
      q(npt)=0.
      enddo
 
      pos=1.
      do i=1,3
      do j=1,6
      npt=i+(j-1)*4
      haut=float(j)/6.
        if(haut.gt.pos)then
        q(npt)=0.
        else
        q(npt)=100.
        endif
      enddo
      enddo

      iter=0
      iconv=0
c
c  initialisation puissance a iter=0
c
      CALL cpeRE(compo,CP_ITERATION, ti, iter, 'puissi', 24,
     &            q , info)
      IF( info.NE. CPOK  )GO TO 9000
 
      do while( iconv .eq. 0)
c
c  lecture de la temperature combustible a iter        
c
      CALL cplRE(compo,CP_ITERATION, ti,tf, iter, 'tempi', 24,
     &      nval, temp , info)
      IF( info.NE. CPOK  )GO TO 9000
 
      do j=1,6
      npt=j*4
      q(npt)=0.
      enddo

c
c  calcul de la puissance degagee en fonction de la position
c    des barres et de la temperature
c
      do i=1,3
      do j=1,6
      npt=i+(j-1)*4
      haut=float(j)/6.
        if(haut.gt.pos)then
        q(npt)=0.
        else
        q(npt)=100.*(1.-0.0001*(temp(npt)-1000.))
        endif
      enddo
      enddo
c
c  ecriture de la puissance a iter+1
c
      iter=iter+1
      CALL cpeRE(compo,CP_ITERATION, ti, iter, 'puissi', 24,
     &            q , info)
      IF( info.NE. CPOK  )GO TO 9000
c
c   lecture du flag de convergence iconv
c
      CALL cplEN(compo,CP_ITERATION,ti, tf, iter  , 'iconv', 1,
     &      nval, iconv  , info)
      write(6,*)"info:",info
      write(6,*)"NEUTRO:",iter,iconv
      CALL FLUSH(6)
      IF( info.NE. CPOK  )GO TO 9000
 
      if(iconv.eq.1)go to 9000

      enddo

9000  continue
      CALL cpfin(compo,CP_ARRET, info)
      end
