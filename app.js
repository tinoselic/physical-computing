const mainMenu = document.querySelector('.mainMenu');
const closeMenu = document.querySelector('.closeMenu');
const openMenu = document.querySelector('.openMenu');

openMenu.addEventListener('click',showNav);
closeMenu.addEventListener('click',closeNav);

function showNav() {
    mainMenu.style.display = 'block';
    mainMenu.style.top = '0';
    openMenu.style.display = 'none';
}

function closeNav() {
    mainMenu.style.display = 'none';
    mainMenu.style.top = '-100%';
    openMenu.style.display = 'block';
}