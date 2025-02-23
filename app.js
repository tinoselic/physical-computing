const mainMenu = document.querySelector('.mainMenu');
const closeMenu = document.querySelector('.closeMenu');
const openMenu = document.querySelector('.openMenu');
const main = document.querySelector('main');

openMenu.addEventListener('click',showNav);
closeMenu.addEventListener('click',closeNav);

function showNav() {
    mainMenu.style.display = 'block';
    openMenu.style.display = 'none';
    main.style.display = 'none';
}

function closeNav() {
    mainMenu.style.display = 'none';
    openMenu.style.display = 'block';
    main.style.display = 'block';
}

// Footer
document.addEventListener("DOMContentLoaded", function() {
    var footer = document.getElementById("copyright");
    var footerText = "© 2025 Tino Selic";
    footer.textContent = footerText;
});